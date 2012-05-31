/*!
 * CaSSiS interface for the ARB PT-Server.
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Parts of the code were taken from: PT_main.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "ptserver.h"

#include <probe.h>
#include <pt_prototypes.h>
#include <arbdbt.h>
//
#include <PT_server_prototypes.h>
#include <BI_basepos.hxx>
#include <arb_file.h>
#include <arb_defs.h>
#include <servercntrl.h>
#include <server.h>
#include <client.h>
#include <struct_man.h>
#include <ut_valgrinded.h>
#include <ptclean.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_TRY 10
#define TIME_OUT 1000*60*60*24

struct ARBPTServer_private {
    char T_or_U;
    PT_exProb *pep;
    PT_local *locs;
    PT_pdc *pdc;
};

/*!
 * These are global variables which handle the PT-Server internal
 * communication. They have to be global due to code compatibility.
 * --> There can only be one PT-Server open at a time.
 */
struct probe_struct_global psg;
int gene_flag;
gene_struct_index_arb gene_struct_arb2internal;
gene_struct_index_internal gene_struct_internal2arb;
PT_main *aisc_main;
//
ULONG physical_memory;
gene_struct_list all_gene_structs;
int aisc_core_on_error;
char *species_name_buf;

/*!
 * Dummy functions, necessary for compatibility to ARB.
 */
int server_shutdown(PT_main*, aisc_string) {
    return 0;
}
int broadcast(PT_main*, int) {
    return 0;
}

ARBPTServer::ARBPTServer() :
                is_computed_flag(false), has_result_buf(false), result_buf(NULL), result_buf_ptr(
                        NULL) {
    // Define a dummy ARBHOME environment variable
    setenv("ARBHOME", ".", 1);

    // Clean-up the psg struct.
    memset((char *) &psg, 0, sizeof(psg));
    for (int i = 0; i < 256; i++) {
        psg.complement[i] = PT_complement(i);
    }

    // Create a new ARB shell.
    psg.gb_shell = new GB_shell;

    // Create a new (temporary) arb database, if not already done...
    psg.gb_main = GB_open("ptserver.arb", "rwc");
    if (psg.gb_main == NULL) {
        GB_await_error();
        GB_clear_error();
        printf("An error occurred while creating a "
                "temporary database.\n");
        return;
    }

    GB_begin_transaction(psg.gb_main);

    // Set database type to 'gene' (we do not yet support genome sequences).
    // GBT_write_int(psg.gb_main, "genom_db", 0);

    // Create a species entry.
    psg.gb_species_data = GB_search(psg.gb_main, "species_data",
            GB_CREATE_CONTAINER);
    if (!psg.gb_species_data) {
        GB_await_error();
        GB_clear_error();
        printf("An error occurred while creating the "
                "'species_data' entry in the temporary database.\n");
        return;
    }

    // Define an alignment definition.
    GBDATA *gb_presets = GB_search(psg.gb_main, "presets", GB_CREATE_CONTAINER);
    GBT_write_string(gb_presets, "use", "ali_temp");
    GBDATA *gb_alignment = GB_search(gb_presets, "alignment",
            GB_CREATE_CONTAINER);
    GBT_write_string(gb_alignment, "alignment_name", "ali_temp");
    GBT_write_string(gb_alignment, "alignment_type", "rna"); // TODO: Predefined as RNA
    GBT_write_int(gb_alignment, "aligned", 0);

    // Create an empty 'extended data' entry...
    psg.gb_sai_data = GB_search(psg.gb_main, "extended_data",
            GB_CREATE_CONTAINER);

    GB_commit_transaction(psg.gb_main);

    // Create a name buffer.
    species_name_buf = (char*) malloc(256);

    // Initialize the ARBPTServer_private struct.
    _priv = (ARBPTServer_private*) calloc(1, sizeof(ARBPTServer_private));
    _priv->T_or_U = 'U';
    _priv->pep = (PT_exProb *) calloc(1, sizeof(PT_exProb));
    _priv->locs = NULL;
    _priv->pdc = NULL;

    // Create new locs entry...
    _priv->locs = (PT_local *) calloc(1, sizeof(PT_local));

    // Dirty hack to get the aisc-commands to do what we want
    _priv->locs->mh.key = KEY_PT_LOCS;
    _priv->locs->ppm.key = KEY_PT_MATCHLIST;
    _priv->locs->ppm.parent = (dllheader_ext*) _priv->locs;

    // Create new pdc entry...
    _priv->pdc = (PT_pdc *) calloc(1, sizeof(PT_pdc));

    // The bond matrix
    static const double bond[4 * 4] = { 0.0, 0.0, 0.5, 1.1, 0.0, 0.0, 1.5, 0.0,
            0.5, 1.5, 0.4, 0.9, 1.1, 0.0, 0.9, 0.0 };

    // Init bonding values...
    for (int i = 0; i < 16; ++i)
        _priv->locs->bond[i].val = bond[i];
    _priv->pdc->dt = 0.5;
    _priv->pdc->dte = 0.5;
    _priv->locs->split = 0.5;

    // Dirty hack to get the aisc-commands to do what we want...
    _priv->locs->pdc = _priv->pdc;
    _priv->locs->ppdc.key = KEY_PT_PDC;
    _priv->locs->ppdc.parent = (dllheader_ext*) _priv->locs;
}

ARBPTServer::~ARBPTServer() {
    if (aisc_main)
        free(aisc_main);

    if (psg.gb_main) {
        delete[] psg.data;

        GB_close(psg.gb_main);
        psg.gb_main = NULL;
        psg.gb_species_data = NULL;
        psg.gb_sai_data = NULL;
    }

    if (psg.gb_shell) {
        delete psg.gb_shell;
        psg.gb_shell = NULL;
    }

    if (psg.namehash)
        GBS_free_hash(psg.namehash);

    free(psg.ecoli);
    delete[] psg.pos_to_weight;
    free(psg.alignment_name);
    free(psg.ptmain);
    free(psg.com_so);

    memset((char *) &psg, 0, sizeof(psg));

    // Free name buffer.
    free(species_name_buf);

    // Free ARBPTServer_private
    free(_priv->pep);
    free(_priv->locs);
    free(_priv->pdc);
    free(_priv);
}

bool ARBPTServer::setTempDir(const char *directory) {
    return false; // TODO: DUMMY!
}

bool ARBPTServer::addSequence(const char *sequence, const id_type id) {
    // Return false: No sequences can be added if the index was computed.
    if (is_computed_flag)
        return false;

    GB_begin_transaction(psg.gb_main);

    GBDATA *gb_species = GB_create_container(psg.gb_species_data, "species");
    if (!gb_species) {
        fprintf(stderr, "An error occurred while creating a "
                "'species' entry in the temporary database.\n");
        GB_commit_transaction(psg.gb_main);
        return false;
    }

    // Add the sequence and alignment to our temporary database.
    snprintf(species_name_buf, 255, "species%d", id);
    GBT_write_string(gb_species, "acc", species_name_buf);
    GBT_write_string(gb_species, "name", species_name_buf);
    GBT_write_string(gb_species, "full_name", species_name_buf);
    GBDATA *gb_ali = GB_create_container(gb_species, "ali_temp");
    GBT_write_string(gb_ali, "data", sequence);

    GB_commit_transaction(psg.gb_main);
    return true;
}

bool ARBPTServer::computeIndex() {
    physical_memory = GB_get_physical_memory();
#if defined(DEBUG)
    printf("physical_memory=%lu k (%lu Mb)\n", physical_memory, physical_memory/1024UL);
#endif // DEBUG
    aisc_main = (PT_main*) calloc(sizeof(PT_main), 1);
    if (!aisc_main)
        return false;
    else {
        aisc_main->key = KEY_PT_MAIN;
        aisc_main->ploc_st.parent = (dllheader_ext *) aisc_main;
        aisc_main->ploc_st.key = KEY_PT_LOCS;
        aisc_main->m_type = 0;
        aisc_main->m_text = strdup("NO_MSG");
        aisc_main->psl.parent = (dllheader_ext *) aisc_main;
        aisc_main->psl.key = KEY_PT_SPECIESLIST;
    }

    ARB_ERROR error;
    GB_set_verbose();

    if (!error) {
        GB_begin_transaction(psg.gb_main);
        psg.alignment_name = GBT_get_default_alignment(psg.gb_main);
        GB_commit_transaction(psg.gb_main);
        printf("Building PT-Server for alignment '%s'...\n",
                psg.alignment_name);
        probe_read_alignments();
        PT_build_species_hash();
    }

    char *pt_name = GBS_global_string_copy("ptserver.pt");
    error = enter_stage_1_build_tree(aisc_main, pt_name);
    if (error) {
        error = GBS_global_string("Failed to build index (Reason: %s)",
                error.deliver());
        printf("%s\n", error.deliver());
    } else {
        char *msg = GBS_global_string_copy(
                "PT_SERVER database \"temp.arb\" has been created.");
        puts(msg);
        free(msg);
    }

    if (error) {
        error = GBS_global_string("Failed to build index (Reason: %s)",
                error.deliver());
        printf("%s\n", error.deliver());
        return false;
    }

    // Clean-up after the build process...
    PTM_finally_free_all_mem();
    free(psg.ptmain);

    // Load/initialize the PT-Server
    error = enter_stage_3_load_tree(aisc_main, pt_name);
    if (error) {
        error = GBS_global_string("Failed to build index (Reason: %s)",
                error.deliver());
        printf("%s\n", error.deliver());
        return false;
    }

    error.expect_no_error();
    is_computed_flag = true;
    return true;
}

bool ARBPTServer::isIndexComputed() {
    return is_computed_flag;

}

bool ARBPTServer::initFetchSignature(unsigned int length, bool RNA) {
    // Hardcoded constraints due the the limitations of the PT-Server.
    if (length < 10 || length > 20)
        return false;

    // Initialize the PT_exProp struct.
    _priv->pep->plength = length;
    _priv->pep->numget = 200;
    _priv->pep->restart = 1;
    if (RNA)
        _priv->T_or_U = 'U';
    else
        _priv->T_or_U = 'T';
    return true;
}

const char *ARBPTServer::fetchNextSignature() {
    static const char *result = NULL;
    static const char *result_ptr = NULL;

    static char *this_result = NULL;
    freenull(this_result);

    if (!result) {
        result = find_next_probe_internal();
        result_ptr = result;

        if (!result)
            return NULL; // Got all probes
    }

    const char *sdot = strchr(result_ptr, ';');
    if (sdot) {
        int len = sdot - result_ptr;
        this_result = (char*) malloc(len + 1);
        memcpy(this_result, result_ptr, len);
        this_result[len] = 0;
        result_ptr = sdot + 1;
    } else {
        this_result = strdup(result_ptr);
        result = result_ptr = 0;
    }
    return this_result;
}

const char *ARBPTServer::find_next_probe_internal() {
    PT_find_exProb(_priv->pep, 0);

    static char *result = NULL;
    freenull(result);
    result = _priv->pep->result;

    if (result && result[0]) {
        for (int j = 0; result[j]; ++j) {
            switch (result[j]) {
            case 2:
                result[j] = 'A';
                break;
            case 3:
                result[j] = 'C';
                break;
            case 4:
                result[j] = 'G';
                break;
            case 5:
                result[j] = 'U';
                break;
            case ';':
                result[j] = ';';
                break;
            default:
                printf("Illegal value (%i) in result ('%s')\n", int(result[j]),
                        result);
                arb_assert(0);
                break;
            }
        }
        return result; // found some probes!
    }
    return NULL;
}

bool ARBPTServer::matchSignature(IntSet *&matched_ids, const char *signature,
        double mm, double mm_dist, unsigned int &og_matches, bool use_wmis) {
    return false; // TODO: DUMMY!
}

bool ARBPTServer::fetchMapping(NameMap &map) {
    return false; // TODO: DUMMY!
}
