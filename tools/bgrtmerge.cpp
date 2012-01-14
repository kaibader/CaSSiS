/*!
 * BGRT Merge Tool
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2011
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

#include "merge.h"

#include <iostream>
#include <cstring>

#include <cassis/bgrt.h>
#include <cassis/namemap.h>
#include <cassis/io.h>

/*!
 * Usage information
 */
void usage() {
    std::cout << "Usage: bgrtmerge <dest-BGRT> <src-BGRTs>...\n"
            "This tool merges two or more BGRTs into a single BGRT file.\n";
}

int main(int argc, char **argv) {
    // Check arguments...
    if (argc < 3 || !strcmp(argv[1], "/?") || !strcmp(argv[1], "-h")
            || !strcmp(argv[1], "--help")) {
        usage();
        return 0;
    }

    // Create destination BGRT file with a dummy size of 1
    // and empty/undefined parameters.
    BgrTree *dest_tree = BgrTree_create(1, true);
    dest_tree->ingroup_mismatch_distance = 99;
    dest_tree->outgroup_mismatch_distance = 99;
    dest_tree->min_gc = 100;
    dest_tree->max_gc = 0;
    dest_tree->min_oligo_len = 999999;
    dest_tree->max_oligo_len = 0;
    dest_tree->min_temp = 273;
    dest_tree->max_temp = -273;

    NameMap dest_map;

    // Merge the BGRTs...
    for (int i = 2; i < argc; ++i) {
        NameMap src_map;
        BgrTree *src_tree = readBGRTFile(&src_map, argv[i]);

        if (!src_tree) {
            std::cout << "Unable to open/read the BGRT file: " << argv[i]
                                                                       << "\n";
            return -1;
        }

        BgrTree_merge(dest_tree, dest_map, src_tree, src_map);
        BgrTree_destroy(src_tree);
    }

    writeBGRTFile(dest_tree, &dest_map, argv[1]);
    return 0;
}
