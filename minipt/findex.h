/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: PT_findEx.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef MINIPT_FINDEX_H
#define MINIPT_FINDEX_H

namespace minipt {

struct bytestring {
    char *data;
    int size;
};

struct PT_exProb { // iterate all existing probes
    int plength; // Length of searched probes
    int numget; // Number of searched probes
    int restart; // 1 => start from beginning
    bytestring next_probe; // next probe to look for (internal)
    char *result; // The result
};

int PT_find_exProb(PT_exProb *pep, int dummy_1x);

} /* namespace minipt */

#endif /* MINIPT_FINDEX_H */
