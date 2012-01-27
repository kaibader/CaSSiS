/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: PT_prefixtree.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef MINIPT_PREFIXTREE_H
#define MINIPT_PREFIXTREE_H

#include <cstdio>

namespace minipt {

void PT_init_count_bits(void);
PTM2 *PT_init(void);
POS_TREE *PT_add_to_chain(PTM2 *ptmain, POS_TREE *node, int name, int apos,
        int rpos);
POS_TREE *PT_change_leaf_to_node(PTM2 *, POS_TREE *node);
POS_TREE *PT_leaf_to_chain(PTM2 *ptmain, POS_TREE *node);
POS_TREE *PT_create_leaf(PTM2 *ptmain, POS_TREE **pfather, PT_BASES base,
        int rpos, int apos, int name);
void PTD_clear_fathers(PTM2 *ptmain, POS_TREE *node);
void PTD_put_longlong(FILE *out, unsigned long i);
void PTD_put_int(FILE *out, unsigned long i);
void PTD_put_short(FILE *out, unsigned long i);
void PTD_debug_nodes(void);
long PTD_write_leafs_to_disk(FILE *out, PTM2 *ptmain, POS_TREE *node, long pos,
        long *pnodepos, int *pblock, bool &error);
bool PTD_read_leafs_from_disk(const char *fname, PTM2 *ptmain,
        POS_TREE **pnode);

} /* namespace minipt */

#endif /* MINIPT_PREFIXTREE_H */
