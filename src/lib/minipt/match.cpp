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
#include "match.h"
#include "io.h"

namespace minipt {

static double ptnd_get_wmismatch(BondingStruct *pdc, char *probe, int pos,
        char ref) {
    int base = probe[pos];
    int complement = ptstruct.complement[base];
    int rowIdx = (complement - (int) PT_A) * 4;
    int maxIdx = rowIdx + base - (int) PT_A;
    int newIdx = rowIdx + ref - (int) PT_A;

    assert(maxIdx >= 0 && maxIdx < 16);
    assert(newIdx >= 0 && newIdx < 16);

    double max_bind = pdc->bond[maxIdx];
    double new_bind = pdc->bond[newIdx];

    return (max_bind - new_bind);
}

inline bool max_number_of_hits_collected(LocalStruct* locs) {
    return locs->pm_max_hits > 0 && locs->pm_hits >= locs->pm_max_hits;
}

void pm_link(PT_probematch *list, PT_probematch *pm) {
    if (!list || !pm)
        return;

    pm->next = NULL;
    if (!list->next) {
        list->next = pm;
        list->last = pm;
    } else {
        list->last->next = pm;
        list->last = pm;
    }
}

struct PT_store_match_in {
    LocalStruct* ilocs;

    PT_store_match_in(LocalStruct* locs) :
        ilocs(locs) {
    }

    int operator()(const DataLoc& matchLoc) {
        // if chain is reached copy data in locs structure

        PT_probematch *ml;
        char *probe = ptstruct.probe;
        int mismatches = ptstruct.mismatches;
        double wmismatches = ptstruct.wmismatches;
        double h;
        int N_mismatches = ptstruct.N_mismatches;
        LocalStruct *locs = (LocalStruct *) ilocs;
        int height;
        int base;
        int ref;
        int pos;
        if (ptstruct.probe) {
            // @@@ code here is a duplicate of code in get_info_about_probe (PT_NT_LEAF-branch)
            pos = matchLoc.rpos + ptstruct.height;
            height = ptstruct.height;
            while ((base = probe[height])
                    && (ref = ptstruct.data[matchLoc.name].data[pos])) {
                if (ref == PT_N || base == PT_N) {
                    // @@@ Warning: dupped code also counts PT_QU as mismatch!
                    N_mismatches++;
                } else if (ref != base) {
                    mismatches++;
                    if (locs->pdc) {
                        h = ptnd_get_wmismatch(locs->pdc, probe, height, ref);
                        wmismatches += ptstruct.pos_to_weight[height] * h;
                    }
                }
                height++;
                pos++;
            }
            while ((base = probe[height])) {
                N_mismatches++;
                height++;
            }
            assert(N_mismatches <= PT_POS_TREE_HEIGHT);
            if (locs->sort_by != PT_MATCH_TYPE_INTEGER) {
                if (ptstruct.w_N_mismatches[N_mismatches]
                                            + (int) (wmismatches + .5) > ptstruct.deep)
                    return 0;
            } else {
                if (ptstruct.w_N_mismatches[N_mismatches] + mismatches
                        > ptstruct.deep)
                    return 0;
            }
        }

        // @@@ dupped code from read_names_and_pos (PT_NT_LEAF-branch)
        ml = (PT_probematch*) calloc(sizeof(*ml), 1);
        ml->name = matchLoc.name;
        ml->b_pos = matchLoc.apos;
        ml->g_pos = -1;
        ml->rpos = matchLoc.rpos;
        ml->wmismatches = wmismatches;
        ml->mismatches = mismatches;
        ml->N_mismatches = N_mismatches;
        ml->sequence = ptstruct.main_probe;
        ml->reversed = ptstruct.reversed ? 1 : 0;

        if (!locs->pm)
            locs->pm = ml;
        else
            pm_link(locs->pm, ml);
        locs->pm_hits++;
        return 0;
    }
};

int read_names_and_pos(LocalStruct *locs, POS_TREE *pt) {
    //! go down the tree to chains and leafs; copy names, positions and mismatches in locs structure

    int base;
    int error;
    int name, pos, rpos;
    PT_probematch *ml;

    if (pt == NULL) {
        return 0;
    }
    if (max_number_of_hits_collected(locs)) {
        locs->matches_truncated = 1;
        return 1;
    }
    if (PT_read_type(pt) == PT_NT_LEAF) {
        name = PT_read_name(ptstruct.ptmain, pt);
        pos = PT_read_apos(ptstruct.ptmain, pt);
        rpos = PT_read_rpos(ptstruct.ptmain, pt);

        // @@@ dupped code from PT_store_match_in::operator()
        ml = (PT_probematch*) calloc(sizeof(*ml), 1);
        ml->name = name;
        ml->b_pos = pos;
        ml->g_pos = -1;
        ml->rpos = rpos;
        ml->mismatches = ptstruct.mismatches;
        ml->wmismatches = ptstruct.wmismatches;
        ml->N_mismatches = ptstruct.N_mismatches;
        ml->sequence = ptstruct.main_probe;
        ml->reversed = ptstruct.reversed ? 1 : 0;

        if (!locs->pm)
            locs->pm = ml;
        else
            pm_link(locs->pm, ml);
        locs->pm_hits++;
        return 0;
    } else {
        if (PT_read_type(pt) == PT_NT_CHAIN) {
            ptstruct.probe = 0;
            if (PT_forwhole_chain(ptstruct.ptmain, pt,
                    PT_store_match_in(locs))) {
                error = 1;
                return 1;
            }
        } else {
            for (base = PT_QU; base < PT_B_MAX; base++) {
                error = read_names_and_pos(
                        locs,
                        PT_read_son_stage_3(ptstruct.ptmain, pt,
                                (PT_BASES) base));
                if (error)
                    return error;
            }

            return 0;
        }
    }
    return 0;
}

int get_info_about_probe(LocalStruct *locs, char *probe, POS_TREE *pt,
        int mismatches, double wmismatches, int N_mismatches, int height) {
    //! search down the tree to find matching species for the given probe

    int name, pos;
    int i;
    int base;
    POS_TREE *pthelp;
    int newmis;
    double newwmis;
    double h;
    int new_N_mis;
    int error;
    if (!pt)
        return 0;
    assert(N_mismatches <= PT_POS_TREE_HEIGHT);
    if (locs->sort_by != PT_MATCH_TYPE_INTEGER) {
        if (ptstruct.w_N_mismatches[N_mismatches] + (int) (wmismatches + 0.5)
                > ptstruct.deep)
            return 0;
    } else {
        if (ptstruct.w_N_mismatches[N_mismatches] + mismatches > ptstruct.deep)
            return 0;
    }
    if (PT_read_type(pt) == PT_NT_NODE && probe[height]) {
        for (i = PT_N; i < PT_B_MAX; i++) {
            if ((pthelp = PT_read_son_stage_3(ptstruct.ptmain, pt, (PT_BASES) i))) {
                new_N_mis = N_mismatches;
                base = probe[height];
                if (base == PT_N || i == PT_N) {
                    newmis = mismatches;
                    newwmis = wmismatches;
                    new_N_mis = N_mismatches + 1;
                } else if (i != base) {
                    if (locs->pdc) {
                        h = ptnd_get_wmismatch(locs->pdc, probe, height, i);
                        newwmis = wmismatches
                                + ptstruct.pos_to_weight[height] * h;
                    } else {
                        newwmis = wmismatches;
                    }
                    newmis = mismatches + 1;
                } else {
                    newmis = mismatches;
                    newwmis = wmismatches;
                }
                error = get_info_about_probe(locs, probe, pthelp, newmis,
                        newwmis, new_N_mis, height + 1);
                if (error)
                    return error;
            }
        }
        return 0;
    }
    ptstruct.mismatches = mismatches;
    ptstruct.wmismatches = wmismatches;
    ptstruct.N_mismatches = N_mismatches;
    if (probe[height]) {
        if (PT_read_type(pt) == PT_NT_LEAF) {
            // @@@ code here is duplicate of code in PT_store_match_in::operator()

            pos = PT_read_rpos(ptstruct.ptmain, pt) + height;
            name = PT_read_name(ptstruct.ptmain, pt);

            // @@@ recursive use of strlen with constant result (argh!)
            if (pos + (int) (strlen(probe + height))
                    >= ptstruct.data[name].size) // end of sequence
                return 0;

            while ((base = probe[height])) {
                i = ptstruct.data[name].data[pos];
                if (i == PT_N || base == PT_N || i == PT_QU || base == PT_QU) {
                    ptstruct.N_mismatches = ptstruct.N_mismatches + 1;
                } else {
                    if (i != base) {
                        ptstruct.mismatches++;
                        if (locs->pdc) {
                            h = ptnd_get_wmismatch(locs->pdc, probe, height, i);
                            ptstruct.wmismatches +=
                                    ptstruct.pos_to_weight[height] * h;
                        }
                    }
                }
                pos++;
                height++;
            }
        } else { // chain
            ptstruct.probe = probe;
            ptstruct.height = height;
            PT_forwhole_chain(ptstruct.ptmain, pt, PT_store_match_in(locs)); // @@@ why ignore result
            return 0;
        }
        assert(ptstruct.N_mismatches <= PT_POS_TREE_HEIGHT);
        if (locs->sort_by != PT_MATCH_TYPE_INTEGER) {
            if (ptstruct.w_N_mismatches[ptstruct.N_mismatches]
                                        + (int) (ptstruct.wmismatches + .5) > ptstruct.deep)
                return 0;
        } else {
            if (ptstruct.w_N_mismatches[ptstruct.N_mismatches]
                                        + ptstruct.mismatches > ptstruct.deep)
                return 0;
        }
    }
    return read_names_and_pos(locs, pt);
}

char *reverse_probe(char *probe, int probe_length) {
    //! mirror a probe

    int i, j;
    char *rev_probe;
    if (!probe_length)
        probe_length = strlen(probe);
    rev_probe = (char *) malloc(probe_length * sizeof(char) + 1);
    j = probe_length - 1;
    for (i = 0; i < probe_length; i++)
        rev_probe[j--] = probe[i];
    rev_probe[probe_length] = '\0';
    return rev_probe;
}
int PT_complement(int base) {
    switch (base) {
    case PT_A:
        return PT_T;
    case PT_C:
        return PT_G;
    case PT_G:
        return PT_C;
    case PT_T:
        return PT_A;
    default:
        return base;
    }
}
void complement_probe(char *probe, int probe_length) {
    //! build the complement of a probe

    int i;
    if (!probe_length)
        probe_length = strlen(probe);
    for (i = 0; i < probe_length; i++) {
        probe[i] = PT_complement(probe[i]);
    }
}

static double calc_position_wmis(int pos, int seq_len, double y1, double y2) {
    return (double) (((double) (pos * (seq_len - 1 - pos))
            / (double) ((seq_len - 1) * (seq_len - 1))) * (double) (y2 * 4.0)
            + y1);
}

void pt_build_pos_to_weight(PT_MATCH_TYPE type, const char *sequence) {
    delete[] ptstruct.pos_to_weight;
    int slen = strlen(sequence);
    ptstruct.pos_to_weight = new double[slen + 1];
    int p;
    for (p = 0; p < slen; p++) {
        if (type == PT_MATCH_TYPE_WEIGHTED_PLUS_POS) {
            ptstruct.pos_to_weight[p] = calc_position_wmis(p, slen, 0.3, 1.0);
        } else {
            ptstruct.pos_to_weight[p] = 1.0;
        }
    }
    ptstruct.pos_to_weight[slen] = 0;
}

int probe_match(LocalStruct *locs, char *probestring) {
    /*! find out where a given probe matches */

    char *rev_pro;

    free(locs->pm_sequence);
    locs->pm_sequence = strdup(probestring);

    ptstruct.main_probe = locs->pm_sequence;

    compress_data(probestring);

    PT_probematch *ml = locs->pm;
    while (ml) {
        PT_probematch *next = ml->next;
        free(ml);
        ml = next;
    }
    locs->matches_truncated = 0;
    locs->pm = NULL;
    locs->pm_hits = 0;

#if defined(DEBUG) && 0
    PT_pdc *pdc = locs->pdc;
    if (pdc) {
        printf("Current bond values:\n");
        for (int y = 0; y<4; y++) {
            for (int x = 0; x<4; x++) {
                printf("%5.2f", pdc->bond[y*4+x].val);
            }
            printf("\n");
        }
    }
#endif // DEBUG
    int probe_len = strlen(probestring);
    if ((probe_len - 2 * locs->pm_max) < MIN_PROBE_LENGTH) {
        if (probe_len >= MIN_PROBE_LENGTH) {
            int max_pos_mismatches = (probe_len - MIN_PROBE_LENGTH) / 2;
            if (max_pos_mismatches > 0) {
                if (max_pos_mismatches > 1) {
                    printf(
                            "Max. %i mismatches are allowed for probes of length %i",
                            max_pos_mismatches, probe_len);
                } else {
                    printf("Max. 1 mismatch is allowed for probes of length %i",
                            probe_len);
                }
            } else {
                printf("No mismatches allowed for that short probes.");
            }
        } else {
            printf("Min. probe length is %i", MIN_PROBE_LENGTH);
        }
        return 0;
    }

    unsigned int ignored_Nmismatches = locs->pm_nmatches_ignored;
    unsigned int when_less_than_Nmismatches = locs->pm_nmatches_limit;

    if ((when_less_than_Nmismatches - 1) > PT_POS_TREE_HEIGHT)
        when_less_than_Nmismatches = PT_POS_TREE_HEIGHT + 1;
    if (ignored_Nmismatches >= when_less_than_Nmismatches)
        ignored_Nmismatches = when_less_than_Nmismatches - 1;

    ptstruct.w_N_mismatches[0] = 0;
    unsigned int mm;
    for (mm = 1; mm < when_less_than_Nmismatches; ++mm) {
        ptstruct.w_N_mismatches[mm] =
                mm > ignored_Nmismatches ? mm - ignored_Nmismatches : 0;
    }
    assert(mm <= (PT_POS_TREE_HEIGHT + 1));
    for (; mm <= PT_POS_TREE_HEIGHT; ++mm) {
        ptstruct.w_N_mismatches[mm] = mm;
    }

    if (locs->pm_complement) {
        complement_probe(probestring, 0);
    }
    ptstruct.reversed = 0;

    free(locs->pm_sequence);
    locs->pm_sequence = strdup(probestring);

    ptstruct.main_probe = locs->pm_sequence;

    ptstruct.deep = locs->pm_max;
    pt_build_pos_to_weight((PT_MATCH_TYPE) locs->sort_by, probestring);

    assert(ptstruct.deep >= 0);
    // deep < 0 was used till [8011] to trigger "new match" (feature unused)
    get_info_about_probe(locs, probestring, ptstruct.pt, 0, 0.0, 0, 0);

    if (locs->pm_reversed) {
        ptstruct.reversed = 1;
        rev_pro = reverse_probe(probestring, 0);
        complement_probe(rev_pro, 0);
        free(locs->pm_csequence);
        locs->pm_csequence = ptstruct.main_probe = strdup(rev_pro);

        get_info_about_probe(locs, rev_pro, ptstruct.pt, 0, 0.0, 0, 0);

        free(rev_pro);
    }
    free(probestring);
    return 0;
}

} /* namespace minipt */
