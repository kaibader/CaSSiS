/*!
 * Simple phylogenetic tree testing tool.
 * Implemented to debug the Newick tree reader.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2014
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

#include <cassis/tree.h>
#include "newick.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    // Check the number of parameters...
    if (argc != 2) {
        printf("Usage:\t%s <NEWICK.TREE>\n"
                "\tIf the import did not fail, "
                "the imported tree is returned to STDOUT.\n", argv[0]);
        return EXIT_SUCCESS;
    }

    // TODO ...
    CaSSiSTree *ctree = Newick2CaSSiSTree(argv[1], 0);

    if (ctree) {
        NameMap mapping;
        ctree->fetchMapping(mapping);
        DumpCaSSiSTree(ctree, mapping);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
