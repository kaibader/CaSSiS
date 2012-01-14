/*!
 * BGRT Info Tool
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

#include <cassis/bgrt.h>
#include <cassis/io.h>

#include <iostream>
#include <cstring>

/*!
 * Usage information
 */
void usage() {
    std::cout << "Usage: bgrtinfo <bgrtfile>\n"
            "This tool provides detailed information "
            "about the specified BGRT file.\n";
}

/*!
 * These are the standard parameters that were used during the
 * BGRT construction process...
 */
void stdInfo(const BgrTree *bgr_tree) {
    std::cout << "Parameters used during the BGRT construction process:"
            "\n\t- Oligo. length: " << bgr_tree->min_oligo_len << "-"
            << bgr_tree->max_oligo_len << " bases"
            "\n\t- Ingroup mismatches: "
            << bgr_tree->ingroup_mismatch_distance
            << "\n\t- Mismatch distance to outgroup: "
            << bgr_tree->outgroup_mismatch_distance << "\n";

    if (bgr_tree->min_gc > 0 || bgr_tree->max_gc < 100)
        std::cout << "\t- G+C range: " << bgr_tree->min_gc << "-"
        << bgr_tree->max_gc << " %\n";

    if (bgr_tree->min_temp > -270 || bgr_tree->max_temp < 270)
        std::cout << "\t- Melting temp. range: " << bgr_tree->min_temp << "-"
        << bgr_tree->max_temp << " °C\n";

    if (bgr_tree->comment)
        std::cout << "\t- Comment: " << bgr_tree->comment << "\n";
}

/*!
 * This function calculates statistical information on the
 * BGRT data structure.
 */
void statisticalInfo(const BgrTree */*bgr_tree*/) {

}

/*!
 * BGRT Info Tool
 */
int main(int argc, char **argv) {
    // Check arguments...
    if (argc != 2 || !strcmp(argv[1], "/?") || !strcmp(argv[1], "-h")
            || !strcmp(argv[1], "--help")) {
        usage();
        return 0;
    }

    // Create a NameMap and open the BGRT file...
    NameMap *name_map = new NameMap();
    BgrTree *bgr_tree = readBGRTFile(name_map, argv[1]);
    if (!bgr_tree) {
        std::cerr << "Error: unable to open/read the BGRT file.\n";
        usage();
        return 1;
    }

    // Show information...
    stdInfo(bgr_tree);

    // Clean-up...
    BgrTree_destroy(bgr_tree);
    delete (name_map);
    return 0;
}
