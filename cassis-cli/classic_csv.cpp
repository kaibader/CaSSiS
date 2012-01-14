/*!
 * Dump the results from a CaSSiSTree into 'classic' CSV tables.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
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

#include "classic_csv.h"

//#include <cassis/bgrt.h>
#include <sstream>
#include <cstdio>
#include <cstdlib>

///*!
// * INTERNAL!
// * Dump the results from a CaSSiSTree into a CSV table.
// *
// * \param tree CaSSiSTree with the processed results.
// * \return true, if successful. Otherwise false.
// */
//static bool dumpNode2ClassicCSV(FILE *fd, CaSSiSTreeNode */*node*/) {
//
//    return false; /* Default parameter. Will change later... */
//}

/*!
 * Dump the results from a CaSSiSTree into a CSV table.
 *
 * \param tree CaSSiSTree with the processed results.
 * \return true, if successful. Otherwise false.
 */
bool dump2ClassicCSV(CaSSiSTree *tree) {
    // Open dump csv file...
    FILE *dumpfile = fopen("result_array.csv", "a+");
    if (!dumpfile)
        return false;

    // Some variables that are worth being fetched...
    CaSSiSTreeNode **node_array = tree->internal_node_array;

    // Create file name header
    fprintf(dumpfile, "ID");
    for (unsigned int n_pos = 0; n_pos < tree->num_nodes; ++n_pos) {
        CaSSiSTreeNode *node = node_array[n_pos];
        std::string name;

        if (node->isLeaf())
            name = tree->leaf_mapping.name(node->this_id);
        else
            name = tree->group_mapping.name(node->this_id);

        if (name.length() > 0)
            fprintf(dumpfile, ",\"%s\"", name.c_str());
        else
            fprintf(dumpfile, ",\"\"");
    }

    // Add the group size
    fprintf(dumpfile, "\nSize");
    for (unsigned int n_pos = 0; n_pos < tree->num_nodes; ++n_pos)
        fprintf(dumpfile, ",%i", node_array[n_pos]->group->size());
    fprintf(dumpfile, "\n");

    // Dump the array into the file. Each row represents a fixed number of
    // outgroup matches, each column one phylogenetic node.
    for (unsigned int outg = 0; outg <= tree->allowed_outgroup_matches;
            outg++) {
        // Dump the number of outgroup matches
        fprintf(dumpfile, "Outgrp.=%i", outg);

        // Dump the number of ingroup matches for each node.
        for (unsigned int n_pos = 0; n_pos < tree->num_nodes; ++n_pos)
            fprintf(dumpfile, ",%i", node_array[n_pos]->num_matches[outg]);

        // Dump newline character
        fprintf(dumpfile, "\n");
    }

    // Close dump-file.
    fclose(dumpfile);

    unsigned int outg = 0;
    while (outg <= tree->allowed_outgroup_matches) {
        // Overall number of signatures in the current CSV file...
        unsigned long overall_num_signatures = 0;

        // Create the filename, based on the number of outgroup hits
        std::ostringstream filename;
        filename << "results_" << outg << ".csv";

        // Open dump csv file...
        dumpfile = fopen(filename.str().c_str(), "a+");
        if (!dumpfile)
            return false;

        // Dump column headers
        fprintf(dumpfile, "Index,ID,Size,Ingroup,Signatures...\n");

        unsigned int i;
        for (i = 0; i < tree->num_nodes; ++i) {
            CaSSiSTreeNode *node = tree->internal_node_array[i];
            unsigned int num_result_entries = node->signatures[outg].size();

            // Only dump nodes with signatures...
            if (num_result_entries > 0) {
                // Dump index
                fprintf(dumpfile, "%i", i);

                // Dump ID
                std::string name;
                if (node->isLeaf())
                    name = tree->leaf_mapping.name(node->this_id);
                else
                    name = tree->group_mapping.name(node->this_id);
                fprintf(dumpfile, ",\"%s\",%i,%i", name.c_str(),
                        node->group->size(), node->num_matches[outg]);

                // Dump signatures
                unsigned int k;
                for (k = 0; k < num_result_entries; ++k)
                    // if (node->signatures[outg].base4_compressed) {
                    //     char *str = entry->signatures.base4->val(k)->toChar(
                    //             true); // TODO: RNA!?
                    //     fprintf(dumpfile, ",%s", str);
                    //     free(str);
                    // } else
                    fprintf(dumpfile, ",%s", node->signatures[outg].val(k));

                fprintf(dumpfile, "\n");
                overall_num_signatures += num_result_entries;
            }
#if DUMP_ALL_NODES
            else {
                if (node) {
                    // Dump all available information...
                    fprintf(dumpfile, "%i,\"%s\",%i,0\n", i,
                            map->name(node->id).c_str(), node->group->size);
                } else {
                    // .. or at least dump the id
                    fprintf(dumpfile, "%i,\"\",0,0\n", i);
                }
            }
#endif
        }

        // Close the dump csv file.
        fclose(dumpfile);

        //
        printf("Overall number of signatures (redundant!) "
                "in result_%d.csv: %ld\n", outg, overall_num_signatures);
        overall_num_signatures = 0;

        // Increase the outgroup hits counter
        ++outg;
    }
    return true;
}
