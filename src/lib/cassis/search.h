/*!
 * BGRT search/traversal functionalities
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2010-2012
 *     Kai Christian Bader <mail@kaibader.de>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BGRT_SEARCH_H_
#define BGRT_SEARCH_H_

#include "bgrt.h"
#include "tree.h"

/*!
 * Computes group specific signatures using the phylogenetic tree structure.
 */
bool findTreeSpecificSignatures(struct BgrTree *bgr_tree,
        const CaSSiSTree *cassis_tree, unsigned int max_outgroup_hits);

/*!
 * Computes group specific signatures based on a given identifier list.
 */
bool findGroupSpecificSignatures(struct BgrTree *bgr_tree, IntSet *ids,
        unsigned int *&num_matches, StrRefSet *&signatures,
        unsigned int max_outgroup_hits);

/*!
 * Computes group specific signatures based on a given node.
 * Caution: This is a HACK!
 */
bool findNodeSpecificSignatures(struct BgrTree *bgr_tree, CaSSiSTreeNode *node,
        unsigned int *&num_matches, unsigned int max_outgroup_hits);

void setNumProcessors(unsigned int n);

#endif /* BGRT_SEARCH_H_ */
