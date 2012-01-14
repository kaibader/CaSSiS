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

#ifndef MINIPT_BUILDTREE_H
#define MINIPT_BUILDTREE_H

namespace minipt {

bool enter_stage_1_build_tree(const char *tname);
bool enter_stage_3_load_tree(const char *tname);

} /* namespace minipt */

#endif /* MINIPT_BUILDTREE_H */
