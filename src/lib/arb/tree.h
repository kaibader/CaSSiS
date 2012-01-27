/*!
 * ARB phylogenetic tree loader for CaSSiS.
 *
 * Copyright (C) 2011,2012
 *     Kai Christian Bader <mail@kaibader.de>
 */

#ifndef ARB_TREE_H_
#define ARB_TREE_H_

#include <cassis/tree.h>

/*!
 * Create a CaSSiSTree based on an ARB database and tree structure.
 *
 * \param arb_db_name Name of the ARB database file
 * \param arb_tree Name of the ARB tree to be fetched/parsed
 * \param allowed_og_matches Number of allowed outgroup matches
 * \return CaSSiSTree, if successful. Otherwise NULL.
 */
CaSSiSTree *fetchTreeFromARB(const char *arb_db_name, const char *arb_tree,
        unsigned int allowed_og_matches = 0);

#endif /* ARB_TREE_H_ */
