/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Parts of the code were taken from: PT_main.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "minipt.h"
#include "probe.h"
#include "match.h"
#include "io.h"
#include "findex.h"
#include "buildtree.h"
#include "prefixtree.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

/*!
 * This function is used to determine the available/free system memory.
 */
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static unsigned long getSystemMemory() {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}
#else // UNIX
#include <unistd.h>
static unsigned long getSystemMemory() {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}
#endif

/*!
 * This is the global struct handling all requests and answers from/to
 * the PT-Server. It has to be global for compatibility to the ARB code.
 * --> There can only be one PT-Server open at at a time.
 */
struct minipt::MiniPTStruct minipt::ptstruct;

unsigned long minipt::physical_memory = 0;

/*!
 * Private entries -- Keeps the header file clean...
 */
class minipt_private {
public:
    minipt_private() :
        r_c_buffer(NULL), r_c_buffer_size(0), find_probe_init(false), pep(
                NULL), locs(NULL), pdc(NULL), RNA(false), filename(
                        strdup("./temp.mpt")) {
    }
    virtual ~minipt_private() {
        free(filename);
    }
    char *r_c_buffer;
    long r_c_buffer_size;
    bool find_probe_init;
    minipt::PT_exProb *pep;
    minipt::LocalStruct *locs;
    minipt::BondingStruct *pdc;
    bool RNA;
    char *filename;
};

MiniPT::MiniPT() :
                _priv(new minipt_private()), is_computed_flag(false), has_result_buf(
                        false), result_buf(NULL), result_buf_ptr(NULL) {
    // Init psg...
    memset((char *) &minipt::ptstruct, 0, sizeof(minipt::MiniPTStruct));
    for (int i = 0; i < 256; ++i)
        minipt::ptstruct.complement[i] = minipt::PT_complement(i);

    // Get available physical memory in bytes!
    minipt::physical_memory = getSystemMemory();

    // Create new locs entry...
    _priv->locs = (minipt::LocalStruct *) calloc(1,
            sizeof(minipt::LocalStruct));

    // Create new pdc entry...
    _priv->pdc = (minipt::BondingStruct *) calloc(1,
            sizeof(minipt::BondingStruct));

    // The bond matrix
    static const double bond[16] = { 0.0, 0.0, 0.5, 1.1, 0.0, 0.0, 1.5, 0.0,
            0.5, 1.5, 0.4, 0.9, 1.1, 0.0, 0.9, 0.0 };

    // Init bonding values...
    for (int i = 0; i < 16; ++i)
        _priv->pdc->bond[i] = bond[i];

    _priv->pdc->dt = 0.5;
    _priv->pdc->dte = 0.5;
    _priv->pdc->split = 0.5;
}

MiniPT::~MiniPT() {
    // Clean-up the PT-Servers data arrays...
    if ((minipt::ptstruct.data_count > 0) && minipt::ptstruct.data) {
        for (unsigned int i = 0; i < minipt::ptstruct.data_count; ++i)
            free(minipt::ptstruct.data[i].data);
    }
    free(minipt::ptstruct.data);

    // Free the reverse complement buffer...
    free(_priv->r_c_buffer);

    // Free the ARB structs #1
    if (_priv->pep) {
        free(_priv->pep->result);
        free(_priv->pep->next_probe.data);
        free(_priv->pep);
    }

    // Iterate through the results and free them...
    if (_priv->locs) {
        minipt::PT_probematch *pm = _priv->locs->pm;
        while (pm) {
            minipt::PT_probematch *pm_next = pm->next;
            free(pm);
            pm = pm_next;
        }
        // Free the ARB structs #2
        free(_priv->locs->pm_sequence);
        free(_priv->locs);
    }

    // Free the ARB structs #3
    free(_priv->pdc);

    // Free the probe compress table
    minipt::cleanup_probe_compress_sequence();

    // Free the e-coli helix stuff...
    delete[] minipt::ptstruct.pos_to_weight;

    // Free ptmain...
    free(minipt::ptstruct.ptmain);

    // Delete private variables
    delete _priv;
}

bool MiniPT::setTempDir(const char *dir) {
    std::string path(dir);
    path = path + "/temp.mpt";
    free(_priv->filename);
    _priv->filename = strdup(path.c_str());
    return true;
}

inline char *probe_append_point(const char *data, int *psize) {
    if (!data) {
        *psize = 0;
        return NULL;
    }

    long len = strlen(data);
    char *buffer;

    if (data[len - 1] != '.') {
        buffer = (char *) malloc(len + 2);
        strcpy(buffer, data);
        buffer[len++] = '.';
    } else {
        buffer = (char *) malloc(len + 1);
        strcpy(buffer, data);
    }
    *psize = len;
    buffer[len] = 0;
    return buffer;
}

bool MiniPT::addSequence(const char *sequence, const id_type id) {
    // Return false: No sequences can be added if the index was computed.
    if (is_computed_flag)
        return false;

    // Increase the counter with the number of imported sequences.
    ++minipt::ptstruct.imported_sequences_counter;

    // Increase the data size (sequence data is stored here), if necessary.
    if (minipt::ptstruct.data_max_size <= minipt::ptstruct.data_count) {
        minipt::ptstruct.data_max_size = minipt::ptstruct.data_count * 2;
        if (minipt::ptstruct.data_max_size == 0)
            minipt::ptstruct.data_max_size = 2;

        minipt::ptstruct.data = (minipt::ProbeDataStruct*) realloc(
                minipt::ptstruct.data,
                minipt::ptstruct.data_max_size
                * sizeof(minipt::ProbeDataStruct));
    }

    // Fetch a reference to the current probe struct entry.
    minipt::ProbeDataStruct& probe_struct =
            minipt::ptstruct.data[minipt::ptstruct.data_count];

    // Set the external ID, if defined. Otherwise use the
    // internal position as ID.
    if (id != (unsigned int) -1)
        probe_struct.id = id;
    else
        probe_struct.id = minipt::ptstruct.data_count;

    // Create a copy of the sequence. Append a '.', if needed.
    int hsize;
    char *data = probe_append_point(sequence, &hsize);

    // pid.checksum = GB_checksum(data, hsize, 1, ".-");
    int size = minipt::probe_compress_sequence(data, hsize);

    probe_struct.data = (char *) malloc(size);
    memcpy(probe_struct.data, data, size);
    probe_struct.size = size;

    free(data);

    minipt::ptstruct.data_count++;
    minipt::ptstruct.char_count += size;
    return true;
}

bool MiniPT::computeIndex() {
    // TODO: Add a 'database initialized' check!

    // PT-Server build process
    bool error = minipt::enter_stage_1_build_tree(_priv->filename);
    if (error)
        return false;

    // Clean-up after the build process...
    free(minipt::ptstruct.ptmain);
    free(minipt::ptstruct.pt);

    // Load/initialize the PT-Server
    error = minipt::enter_stage_3_load_tree(_priv->filename);

    // TODO: DEBUG
    printf("Database contains %i sequences\n", minipt::ptstruct.data_count);
    minipt::PTD_debug_nodes();

    // Returns true, if successfully computed. Otherwise false...
    if (!error)
        is_computed_flag = true;
    return is_computed_flag;
}

bool MiniPT::isIndexComputed() {
    return is_computed_flag;
}

bool MiniPT::initFetchSignature(unsigned int length, bool RNA) {
    // Init with default values...
    _priv->RNA = RNA;
    if (!_priv->pep)
        _priv->pep = (minipt::PT_exProb *) calloc(1, sizeof(minipt::PT_exProb));
    _priv->pep->plength = length;
    _priv->pep->numget = 200;
    _priv->pep->restart = 1;

    has_result_buf = false;
    result_buf = NULL;
    result_buf_ptr = NULL;

    return true;
}

const char *MiniPT::fetchNextSignature() {
    if (!has_result_buf) {
        free(result_buf);

        PT_find_exProb(_priv->pep, 0);
        result_buf = _priv->pep->result;

        if (result_buf && result_buf[0]) {
            for (int j = 0; result_buf[j]; ++j) {
                switch (result_buf[j]) {
                case 2:
                    result_buf[j] = 'A';
                    break;
                case 3:
                    result_buf[j] = 'C';
                    break;
                case 4:
                    result_buf[j] = 'G';
                    break;
                case 5:
                    result_buf[j] = _priv->RNA ? 'U' : 'T';
                    break;
                case ';':
                    result_buf[j] = ';';
                    break;
                default:
                    printf("Illegal value (%i) in result ('%s')\n",
                            int(result_buf[j]), result_buf);
                    // Trigger a SegFault.
                    assert(0);
                    break;
                }
            }
        }

        if (!result_buf || (result_buf[0] == 0))
            return NULL; // All signatures were fetched.

        has_result_buf = true;
        result_buf_ptr = result_buf;
    }

    char *semicolon = strchr(result_buf_ptr, ';');
    const char *result = result_buf_ptr;
    if (semicolon) {
        *semicolon = 0;
        result_buf_ptr = semicolon + 1;
    } else {
        has_result_buf = false;
    }

    return result;
}

bool MiniPT::matchSignature(IntSet *&matched_ids, const char *signature,
        double mm, double mm_dist, unsigned int &og_matches, bool use_wmis) {
    // Set mismatch parameter, depending on the allowed ingroup mismatches and
    // the mismatch distance to outgroup hits.
    // TODO: Is "mm_dist = mm + 1" also correct for weighted mismatches?
    if (mm < 0)
        mm = 0;
    if (mm_dist < mm)
        mm_dist = mm + 1.0;

    // mismatches = mm_dist + 0.5 (for wmis) - 1 (because a distance of 1 is default)
    unsigned int mismatches = (unsigned int) (mm_dist - 0.5);

    // Set conditions under which the probe should match...
    _priv->locs->pm_reversed = 0; // Match reverse probe (0 = off)
    _priv->locs->pm_complement = 0; // Match complement probe (0 = off)
    _priv->locs->pm_max = mismatches; // Max. number of mismatches
    _priv->locs->pm_max_hits = 0; // max. number of reported hits (0 = unlimited)
    _priv->locs->sort_by = 0; // 0 == mismatches
    _priv->locs->pm_nmatches_ignored = 1; // Max. of accepted matches vs. N
    _priv->locs->pm_nmatches_limit = 4; // N-matches are only accepted, if less than NMATCHES_LIMIT occur, otherwise no N-matches are accepted

    // Match probe string against the PT-Server
    // (Note: 'buf' will be freed by probe_match())
    char *buf = (char *) malloc(strlen(signature) + 1);
    strcpy(buf, signature);
    probe_match(_priv->locs, buf);

    // Create an empty IntSet, if necessary. Otherwise just clean it.
    if (matched_ids == NULL) {
        matched_ids = new IntSet();
    } else {
        matched_ids->clear();
    }
    og_matches = 0;

    // Iterate through the results...
    minipt::PT_probematch *pm;
    for (pm = _priv->locs->pm; pm; pm = pm->next) {
        if (use_wmis) {
            // Evaluate the results based on their weighted mismatch values.
            // Ignore signatures with wmis-scores above mm_dist.
            if (pm->wmismatches <= mm_dist) {
                if (pm->wmismatches > mm) {
                    // Count match to the 'supposed outgroup'.
                    ++og_matches;
                } else {
                    // Add the matched sequence to our result set.
                    matched_ids->add(minipt::ptstruct.data[pm->name].id);
                }
            }
        } else {
            // Evaluate the results based on their regular mismatch values.
            if (pm->mismatches > mm) {
                // Count match to the 'supposed outgroup'.
                ++og_matches;
            } else {
                // Add the matched sequence to our result set.
                matched_ids->add(minipt::ptstruct.data[pm->name].id);
            }
        }
    }
    return true;
}
