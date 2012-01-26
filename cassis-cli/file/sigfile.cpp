/*!
 * Dump the results from a CaSSiSTree into separate signature files.
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

#include "sigfile.h"

#include <cassis/thermodynamics.h>

#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

/*!
 * Shortens a double value (the precision). Quick-n-dirty style...
 *
 * \param value Number that should be truncated.
 * \param digits Precision.
 */
inline double precision(double value, unsigned int digits) {
    unsigned int d = digits * 10;
    return (double) ((long) (value * d)) / 10;
}

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

    // Create a base for thermodynamic calculations.
    Thermodynamics therm;

    unsigned int outg = 0;
    while (outg <= og_matches) {
        unsigned int num_result_entries = node->signatures[outg].size();

        // Only dump nodes with an id and valid signatures.
        // Dump one signature per line.
        if (num_result_entries > 0) {
            if (og_matches > 0) {
                stream << "\n--------------------- Signatures with " << outg;
                if (outg == 1)
                    stream << " outgroup match\n";
                else
                    stream << " outgroup matches\n";
            }

            for (unsigned int k = 0; k < num_result_entries; ++k) {
                // Fetch signature and process it.
                const char *signature = node->signatures[outg].val(k);
                therm.process(signature);

                // Output information about the signature. #1
                stream << "\nSignature:            3'-" << signature << "-5'\n"
                        << "Length:               " << strlen(signature)
                        << " nt\n";

                // Output information about the signature, if it is a group. #2
                if (!node->isLeaf())
                    stream
                    << "Coverage:             "
                    << node->num_matches[outg]
                                         << " of "
                                         << node->group->size()
                                         << " ("
                                         << precision(
                                                 (double) node->num_matches[outg] * 100
                                                 / node->group->size(), 1) << "%)\n";

                // Output information about the signature. #3
                stream << "Outgroup matches:     " << outg << "\n"
                        << "G+C Content:          "
                        << precision(therm.get_gc_content(), 1) << "%\n"
                        << "T_m (basic):          "
                        << precision(therm.get_tm_basic(), 1) << "°C\n"
                        << "T_m (base stacking):  "
                        << precision(therm.get_tm_base_stacking(), 1) << "°C\n"
                        << "Entropy (dS):         "
                        << precision(therm.get_delta_s(), 1) << " cal/(mol*K)\n"
                        << "Enthalpy (dH):        "
                        << precision(therm.get_delta_h(), 1) << " kcal/mol\n"
                        << "Gibbs f.e. (dG 37°C): "
                        << precision(therm.get_delta_g37(), 1) << " kcal/mol\n";
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
            std::string filename(name);
            size_t pos =
                    filename.find_first_not_of(
                            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_");
            while (pos != std::string::npos) {
                filename[pos] = '_';
                pos =
                        filename.find_first_not_of(
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_",
                                pos + 1);
            }

            // Create/open file, write the results, and close it.
            filename.append(".sig");
            std::ofstream file;
            file.open(filename.c_str());

            // Write a header (info about the current leaf/group)
            if (node->isLeaf())
                file << "Organism:             " << name << "\n";
            else
                file << "Group name:           " << name << "\n"
                << "Group size:           " << node->group->size()
                << "\n";

            dump2stream(file, node, iface, tree->allowed_outgroup_matches);
            file.close();
        }
    }
    return returnvalue;
}
