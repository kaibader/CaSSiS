/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: PT_match.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "probe.h"
#include "probe-tree.h"
#include "buildtree.h"
#include "prefixtree.h"

#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace minipt {

POS_TREE *build_pos_tree(POS_TREE *pt, int anfangs_pos, int apos, int RNS_nr,
        unsigned int end) {
    static POS_TREE *pthelp, *pt_next;
    unsigned int i, j;
    int height = 0, anfangs_apos_ref, anfangs_rpos_ref, RNS_nr_ref;
    pthelp = pt;
    i = anfangs_pos;
    while (PT_read_type(pthelp) == PT_NT_NODE) { // now we got an inner node
        if ((pt_next = PT_read_son_stage_1(ptstruct.ptmain, pthelp,
                (PT_BASES) ptstruct.data[RNS_nr].data[i])) == NULL) {
            // there is no son of that type -> simply add the new son to that path //
            if (pthelp == pt) { // now we create a new root structure (size will change)
                PT_create_leaf(ptstruct.ptmain, &pthelp,
                        (PT_BASES) ptstruct.data[RNS_nr].data[i], anfangs_pos,
                        apos, RNS_nr);
                return pthelp; // return the new root
            } else {
                PT_create_leaf(ptstruct.ptmain, &pthelp,
                        (PT_BASES) ptstruct.data[RNS_nr].data[i], anfangs_pos,
                        apos, RNS_nr);
                return pt; // return the old root
            }
        } else { // go down the tree
            pthelp = pt_next;
            height++;
            i++;
            if (i >= end) { // end of sequence reached -> change node to chain and add
                // should never be reached, because of the terminal symbol
                // of each sequence
                if (PT_read_type(pthelp) == PT_NT_CHAIN)
                    pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr,
                            apos, anfangs_pos);
                // if type == node then forget it
                return pt;
            }
        }
    }
    // type == leaf or chain
    if (PT_read_type(pthelp) == PT_NT_CHAIN) { // old chain reached
        pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr, apos,
                anfangs_pos);
        return pt;
    }
    anfangs_rpos_ref = PT_read_rpos(ptstruct.ptmain, pthelp); // change leave to node and create two sons
    anfangs_apos_ref = PT_read_apos(ptstruct.ptmain, pthelp);
    RNS_nr_ref = PT_read_name(ptstruct.ptmain, pthelp);
    j = anfangs_rpos_ref + height;

    while (ptstruct.data[RNS_nr].data[i] == ptstruct.data[RNS_nr_ref].data[j]) { // creates nodes until sequences are different
        // type != nt_node
        if (PT_read_type(pthelp) == PT_NT_CHAIN) { // break
            pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr, apos,
                    anfangs_pos);
            return pt;
        }
        if (height >= PT_POS_TREE_HEIGHT) {
            if (PT_read_type(pthelp) == PT_NT_LEAF) {
                pthelp = PT_leaf_to_chain(ptstruct.ptmain, pthelp);
            }
            pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr, apos,
                    anfangs_pos);
            return pt;
        }
        if (((i + 1) >= end)
                && (j + 1 >= (unsigned) (ptstruct.data[RNS_nr_ref].size))) { // end of both sequences
            return pt;
        }
        pthelp = PT_change_leaf_to_node(ptstruct.ptmain, pthelp); // change tip to node and append two new leafs
        if (i + 1 >= end) { // end of source sequence reached
            pthelp = PT_create_leaf(ptstruct.ptmain, &pthelp,
                    (PT_BASES) ptstruct.data[RNS_nr_ref].data[j],
                    anfangs_rpos_ref, anfangs_apos_ref, RNS_nr_ref);
            return pt;
        }
        if (j + 1 >= (unsigned) (ptstruct.data[RNS_nr_ref].size)) { // end of reference sequence
            pthelp = PT_create_leaf(ptstruct.ptmain, &pthelp,
                    (PT_BASES) ptstruct.data[RNS_nr].data[i], anfangs_pos, apos,
                    RNS_nr);
            return pt;
        }
        pthelp = PT_create_leaf(ptstruct.ptmain, &pthelp,
                (PT_BASES) ptstruct.data[RNS_nr].data[i], anfangs_rpos_ref,
                anfangs_apos_ref, RNS_nr_ref);
        // dummy leaf just to create a new node; may become a chain
        i++;
        j++;
        height++;
    }
    if (height >= PT_POS_TREE_HEIGHT) {
        if (PT_read_type(pthelp) == PT_NT_LEAF)
            pthelp = PT_leaf_to_chain(ptstruct.ptmain, pthelp);
        pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr, apos,
                anfangs_pos);
        return pt;
    }
    if (PT_read_type(pthelp) == PT_NT_CHAIN) {
        pthelp = PT_add_to_chain(ptstruct.ptmain, pthelp, RNS_nr, apos,
                anfangs_pos);
    } else {
        pthelp = PT_change_leaf_to_node(ptstruct.ptmain, pthelp); // Blatt loeschen
        PT_create_leaf(ptstruct.ptmain, &pthelp,
                (PT_BASES) ptstruct.data[RNS_nr].data[i], anfangs_pos, apos,
                RNS_nr); // zwei neue Blaetter
        PT_create_leaf(ptstruct.ptmain, &pthelp,
                (PT_BASES) ptstruct.data[RNS_nr_ref].data[j], anfangs_rpos_ref,
                anfangs_apos_ref, RNS_nr_ref);
    }
    return pt;
}

inline void get_abs_align_pos(char *seq, int &pos) {
    // get the absolute alignment position
    int q_exists = 0;
    if (pos > 3) {
        pos -= 3;
        while (pos > 0) {
            uint32_t c = *((uint32_t*) (seq + pos)); // FIXME: Causes invalid read
            if (c == 0x00000000) {
                q_exists = 1;
                pos -= 4;
                continue;
            }
            break;
        }
        pos += 3;
    }
    while (pos) {
        unsigned char c = seq[pos]; // FIXME: Causes invalid read
        if (c == 0x00) {
            q_exists = 1;
            pos--;
            continue;
        }
        break;
    }
    pos += q_exists;
}

long PTD_save_partial_tree(FILE *out, PTM2 *ptmain, POS_TREE * node,
        char *partstring, int partsize, long pos, long *ppos, bool &error) {
    if (partsize) {
        POS_TREE *son = PT_read_son_stage_1(ptmain, node,
                (enum PT_bases_enum) partstring[0]);
        if (son) {
            pos = PTD_save_partial_tree(out, ptmain, son, partstring + 1,
                    partsize - 1, pos, ppos, error);
        }
    } else {
        PTD_clear_fathers(ptmain, node);
        long r_pos;
        int blocked;
        blocked = 1;
        while (blocked && !error) {
            blocked = 0;
            printf("Flushing to disk [%li]\n", pos);
            fflush(stdout);
            r_pos = PTD_write_leafs_to_disk(out, ptmain, node, pos, ppos,
                    &blocked, error);
            if (r_pos > pos)
                pos = r_pos;
        }
    }
    return pos;
}

inline int ptd_string_shorter_than(const char *s, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (!s[i]) {
            return 1;
        }
    }
    return 0;
}

static void PT_init_base_string_counter(char *str, char initval, int size) {
    memset(str, initval, size + 1);
    str[size] = 0;
}

static void PT_inc_base_string_count(char *str, char initval, char maxval,
        int size) {
    int i;
    if (!size) {
        str[0] = 0xff;
        return;
    }
    for (i = size - 1; i >= 0; i--) {
        str[i]++;
        if (str[i] >= maxval) {
            str[i] = initval;
            if (!i)
                str[i] = 0xff; // end flag
        } else {
            break;
        }
    }
}

#define PT_base_string_counter_eof(str) (*(unsigned char *)(str) == 255)

bool enter_stage_1_build_tree(const char *tname) { // __ATTR__USERESULT
    // initialize tree and call the build pos tree procedure

    // ARB_ERROR error;
    bool error_occurred = false;

    if (!error_occurred) {
        FILE *out = fopen(tname, "w");
        if (!out) {
            printf("Cannot open %s\n", tname);
            error_occurred = true;
        } else {
            POS_TREE *pt = NULL;

            putc(0, out); // disable zero father
            long pos = 1;

            ptstruct.ptmain = PT_init();
            ptstruct.ptmain->stage1 = 1; // enter stage 1

            pt = PT_create_leaf(ptstruct.ptmain, NULL, PT_N, 0, 0, 0); // create main node
            pt = PT_change_leaf_to_node(ptstruct.ptmain, pt);
            ptstruct.stat.cut_offs = 0; // statistic information

            long last_obj = 0;
            char partstring[256];
            int partsize = 0;
            int pass0 = 0;
            int passes = 1;

            unsigned long total_size = ptstruct.char_count;

            printf("Overall bases: %li\n", total_size);

            while (1) {
#ifdef ARB_64
                unsigned long estimated_kb = (total_size / 1024) * 55; // value by try and error (for gene server)
#else
                unsigned long estimated_kb = (total_size / 1024) * 35; // value by try and error; 35 bytes per base
#endif
                printf("Estimated memory usage for %i pass(es): %lu MiB. "
                        "(%lu MiB available.)\n", passes, estimated_kb / 1024,
                        physical_memory / (1024 * 1024));

                if (estimated_kb <= (physical_memory / 1024))
                    break;

                total_size /= 4;
                partsize++;
                passes *= 5;
            }

            printf("Tree Build: %li bases in %i pass(es).\n",
                    ptstruct.char_count, passes);

            PT_init_base_string_counter(partstring, PT_N, partsize);
            while (!PT_base_string_counter_eof(partstring)) {
                ++pass0;
                printf("\n Starting pass %i(%i)\n", pass0, passes);

                for (unsigned int i = 0; i < ptstruct.data_count; i++) {
                    int psize = ptstruct.data[i].size;
                    char *align_abs = (char *) malloc(ptstruct.data[i].size);
                    memcpy(align_abs, ptstruct.data[i].data,
                            ptstruct.data[i].size);

                    if ((i % 1000) == 0) {
                        printf("%i(%i)\t cutoffs:%i\n", i, ptstruct.data_count,
                                ptstruct.stat.cut_offs);
                        fflush(stdout);
                    }

                    int abs_align_pos = psize - 1;
                    for (int j = ptstruct.data[i].size - 1; j >= 0;
                            j--, abs_align_pos--) {
                        get_abs_align_pos(align_abs, abs_align_pos); // may result in neg. abs_align_pos (seems to happen if sequences are short < 214bp )
                        if (abs_align_pos < 0)
                            break; // -> in this case abort

                        if (partsize
                                && (*partstring != ptstruct.data[i].data[j]
                                                                         || memcmp(partstring,
                                                                                 ptstruct.data[i].data + j,
                                                                                 partsize)))
                            continue;
                        if (ptd_string_shorter_than(ptstruct.data[i].data + j,
                                9))
                            continue;

                        pt = build_pos_tree(pt, j, abs_align_pos, i,
                                ptstruct.data[i].size);
                    }
                    free(align_abs);
                }
                pos = PTD_save_partial_tree(out, ptstruct.ptmain, pt,
                        partstring, partsize, pos, &last_obj, error_occurred);
                if (error_occurred)
                    break;

                PT_inc_base_string_count(partstring, PT_N, PT_B_MAX, partsize);
            }

            if (!error_occurred) {
                if (partsize) {
                    pos = PTD_save_partial_tree(out, ptstruct.ptmain, pt, NULL,
                            0, pos, &last_obj, error_occurred);
                }
            }
            if (!error_occurred) {
                bool need64bit = false; // does created db need a 64bit ptserver ?
#ifdef ARB_64
                if (last_obj >= 0xffffffff) need64bit = true; // last_obj is bigger than int
#else
                if (last_obj <= 0) { // overflow ?
                    printf("Overflow - out of memory");
                }
#endif

                // write information about database
                long info_pos = pos;
                PTD_put_int(out, PT_SERVER_MAGIC); // marker to identify PT-Server file
                fputc(PT_SERVER_VERSION, out); // version of PT-Server file
                pos += 4 + 1;

                // as last element of info block, write it's size (2byte)
                long info_size = pos - info_pos;
                PTD_put_short(out, info_size);
                pos += 2;

                // save DB footer (which is the entry point on load)

                if (need64bit) { // last_obj is bigger than int
#ifdef ARB_64
                    PTD_put_longlong(out, last_obj); // write last_obj as long long (64 bit)
                    PTD_put_int(out, 0xffffffff);// write 0xffffffff at the end to signalize 64bit ptserver is needed
#else
                    assert(0);
#endif
                } else {
                    PTD_put_int(out, last_obj); // last_obj fits into an int -> store it as usual (compatible to old unversioned format)
                }
            }

            fclose(out);
            ptstruct.pt = pt;
        }
    }
    return error_occurred;
}

bool enter_stage_3_load_tree(const char *tname) { // __ATTR__USERESULT
    // load tree from disk
    bool error = false;

    ptstruct.ptmain = PT_init();
    ptstruct.ptmain->stage3 = 1; // enter stage 3

    struct stat gb_global_stt;
    stat(tname, &gb_global_stt);
    long size = gb_global_stt.st_size;

    if (size < 0) {
        error = true;
    } else {
        printf("Reading PT-Server \"%s\" with size %li from disk.\n", tname,
                size);
        // TODO: REDUNDANT: FILE *in is not further used (but opened again)!
        FILE *in = fopen(tname, "r");
        if (!in) {
            error = true;
        } else {
            error = PTD_read_leafs_from_disk(tname, ptstruct.ptmain,
                    &ptstruct.pt);
            fclose(in);
        }
    }

    return error;
}

} /* namespace minipt */
