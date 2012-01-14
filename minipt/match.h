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

#ifndef MINIPT_MATCH_H
#define MINIPT_MATCH_H

namespace minipt {

struct BondingStruct { // probe design
    double bond[4 * 4]; // The bond matrix
    double dte; // The Temperature drop per percent mismatch on the edge of the probe
    double dt; // The temperature drop per percent mismatch
    double split; // Split the domains if bond value is less the average bond value - split
};

struct LocalStruct { // local communication buffer
    char *pm_sequence; // the sequence
    char *pm_csequence; // the complement sequence
    int pm_reversed; // reverse probe
    int pm_complement; // complement probe
    int pm_max; // max mismatches
    int pm_max_hits; // max number of hits reported (0=unlimited)
    int sort_by; // 0 == mismatches  1 == weighted mismatches 2 == weighted mismatches with pos and strength
    int pm_nmatches_ignored; // max. of accepted matches vs N
    int pm_nmatches_limit; // N-matches are only accepted, if less than NMATCHES_LIMIT occur, otherwise no N-matches are accepted
    PT_probematch *pm; // result: List of species where probe matches
    int pm_hits; // number of hits (= number of entries in pm)
    int matches_truncated; // result: whether MATCH_LIST was truncated
    // int group_count; // result: Number of selected species // TODO: DEPRECATED???
    BondingStruct *pdc; // The new probe design
    // PT_exProb *ep; // Find all existing probes // TODO: DEPRECATED???
};

int PT_complement(int base);
int probe_match(LocalStruct *locs, char *probestring);

} /* namespace minipt */

#endif /* MINIPT_MATCH_H */
