/*!
 * Dump the results from a CaSSiSTree into CSV tables.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
 *
 * Copyright (C) 2012
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

#include "txtfiles.h"

#include <cassis/thermodynamics.h>

#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

/*!
 * Dump the results from a CaSSiSTreeNode onto an output stream.
 *
 * \param ostream Output stream.
 * \param node CaSSiSTreeNode with the processed results.
 * \param iface Search index interface, for detailed match list.
 * \param og_matches Max. number of allowed outgroup matches.
 * \return true, if successful. Otherwise false.
 */
bool dump2stream(std::ostream &stream, CaSSiSTreeNode *node,
        IndexInterface *iface, unsigned int og_matches) {
    // if (node == NULL || iface == NULL)
    if (node == NULL)
        return false;

    // Write a column header
    stream << "\"Size\",\"Coverage\",\"G+C\","
            "\"Tm_basic\",\"Tm_37\",\"Signature\"\n";

    unsigned int outg = 0;
    while (outg <= og_matches) {
        // Create a base for thermodynamic calculations.
        Thermodynamics therm;

        unsigned int num_result_entries = node->signatures[outg].size();

        // Only dump nodes with an id and valid signatures.
        // Dump one signature per line.
        if (num_result_entries > 0) {
            for (unsigned int k = 0; k < num_result_entries; ++k) {
                // Fetch signature and process it.
                const char *signature = node->signatures[outg].val(k);
                therm.process(signature);

                // Dump results in CSV format.
                stream << node->group->size() << node->num_matches[outg]
                        << therm.get_gc_content() << therm.get_tm_basic()
                        << therm.get_tm_base_stacking() << signature << "\n";
            }
        }
        // Increase the outgroup hits counter
        ++outg;
    }

    return true;
}

/*!
 * Dump the results from a CaSSiSTree into multiple CSV tables.
 *
 * \param tree CaSSiSTree with the processed results.
 * \param iface Search index interface, for detailed match list.
 * \return true, if successful. Otherwise false.
 */
bool dump2Textfiles(CaSSiSTree *tree, IndexInterface *iface) {
    // if (tree == NULL || iface == NULL)
    if (tree == NULL)
        return false;

    // Our return value. Needed later.
    bool returnvalue = true;

    // Iterate through all tree nodes.
    for (unsigned int i = 0; i < tree->num_nodes; ++i) {
        CaSSiSTreeNode *node = tree->internal_node_array[i];

        // Fetch the nodes' name, if available.
        // Use it to create a filename.
        std::string name;
        if (node->isLeaf())
            name = tree->leaf_mapping.name(node->this_id);
        else
            name = tree->group_mapping.name(node->this_id);
        if (name.length() > 0) {
            // Replace whitespace characters... dirty, I know...
            size_t pos =
                    name.find_first_not_of(
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_");
            while (pos != std::string::npos) {
                name[pos] = '_';
                pos =
                        name.find_first_not_of(
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_",
                                pos + 1);
            }

            // Create/open file, write the results, and close it.
            name.append(".sig");
            std::ofstream file;
            file.open(name.c_str());
            dump2stream(file, node, iface, tree->allowed_outgroup_matches);
            file.close();
        }
    }
    return returnvalue;
}
