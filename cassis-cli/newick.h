/*!
 * Newick formatted tree file reader
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
 *
 * Copyright (C) 2010,2011
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

#ifndef NEWICK_H_
#define NEWICK_H_

#include <cassis/tree.h>
#include <cassis/namemap.h>

///*!
// * Create/read a phylogenetic tree from a Newick tree file.
// *
// * \param filename String containing the name of the Newick file.
// * \return PhyloTree structure, if successful. Otherwise NULL.
// */
//struct PhyloTree *ParseNewickFile(const char *filename, NameMap &map);

/*!
 * Dump a phylogenetic tree in the Newick format to stdout.
 *
 * \param tree Phylogenetic tree
 */
void DumpCaSSiSTree(const CaSSiSTree *tree, NameMap &map);

/*!
 * Create/read a CaSSiS tree from a Newick tree file.
 *
 * \param filename String containing the name of the Newick file.
 * \return CaSSiSTree structure, if successful. Otherwise NULL.
 */
CaSSiSTree *Newick2CaSSiSTree(const char *filename, unsigned int og_limit = 0);

#endif /* NEWICK_H_ */
