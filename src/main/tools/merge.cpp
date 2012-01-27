/*!
 * BGRT Info Tool
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2011,2012
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

#include <cassis/bgrt.h>

#include <cstdlib>
#include <cstring>
#include <map>
#include <stack>
#include <string>

/*!
 * Merges the names from 'src' into 'dest'. Names, that are appended to 'dest'
 * will have new corresponding identifiers.
 */
std::map<unsigned int, unsigned int> NameMap_merge(NameMap &dest,
        const NameMap &src) {

    // Create src -> dest mapping
    std::map<unsigned int, unsigned int> src2dest_mapping;

    // Go through all entries in src...
    for (unsigned int src_id = 0; src_id < src.size(); src_id++) {
        // Append the name at position 'src_id' to 'dest'.
        // Name mappings are unique: if src.name is already in dest,
        // the current dest.id is returned instead of a new id.
        unsigned int dest_id = dest.append(src.name(src_id));

        // Update the id mapping list...
        src2dest_mapping[src_id] = dest_id;
    }

    return src2dest_mapping;
}

/*!
 * Merge BGRT parameters (integrate from src into dest)
 */
void mergeBGRTParams(BgrTree *dest, const BgrTree *src) {
    if (dest->ingroup_mismatch_distance > src->ingroup_mismatch_distance)
        dest->ingroup_mismatch_distance = src->ingroup_mismatch_distance;

    if (dest->outgroup_mismatch_distance > src->outgroup_mismatch_distance)
        dest->outgroup_mismatch_distance = src->outgroup_mismatch_distance;

    if (dest->min_oligo_len > src->min_oligo_len)
        dest->min_oligo_len = src->min_oligo_len;
    if (dest->max_oligo_len < src->max_oligo_len)
        dest->max_oligo_len = src->max_oligo_len;

    if (dest->min_gc > src->min_gc)
        dest->min_gc = src->min_gc;
    if (dest->max_gc < src->max_gc)
        dest->max_gc = src->max_gc;

    if (dest->min_temp > src->min_temp)
        dest->min_temp = src->min_temp;
    if (dest->max_temp < src->max_temp)
        dest->max_temp = src->max_temp;

    std::string comment;
    if (dest->comment)
        comment.append(dest->comment);
    if (comment.length() > 0)
        comment.append(". ");
    comment.append(src->comment);
    dest->comment = strdup(comment.c_str());
}

///*!
// * Integrates all BGRT nodes from the 'src' tree into the 'dest' tree.
// * This assumes that all identifiers from src and dest are identical and
// * src #identifiers < dest #identifiers.
// *
// * \param dest_bgrt Destination BGRT
// * \param src_bgrt Source BGRT
// * \return Number of copied nodes. Zero if an error occurred.
// */
//unsigned int BgrTree_merge(struct BgrTree *dest_bgrt,
//        const struct BgrTree *src_bgrt) {
//    unsigned int copied_nodes = 0;
//
//    // Check parameters...
//    if (!dest_bgrt || !src_bgrt)
//        return 0;
//
//    return copied_nodes;
//}

/*!
 * Integrates all BGRT nodes from the 'src' tree into the 'dest' tree.
 *
 * \param dest_bgrt Destination BGRT
 * \param dest_map Destination NameMap
 * \param src_bgrt Source BGRT
 * \param src_map Source NameMap
 * \return Number of copied nodes. Zero if an error occurred.
 *
 */
unsigned int BgrTree_merge(struct BgrTree *dest_bgrt, NameMap &dest_map,
        const struct BgrTree *src_bgrt, const NameMap &src_map) {
    unsigned int copied_nodes = 0;

    // Check parameters...
    if (!dest_bgrt || !src_bgrt || src_map.size() == 0)
        return 0;

    // Step 1: integrate the identifiers from 'src_map' into the 'dest_map'.
    std::map<unsigned int, unsigned int> src2dest_mapping = NameMap_merge(
            dest_map, src_map);

    // TODO: Step 2: Adapt the root node array of dest.
    unsigned int dest_map_size = dest_map.size();
    if (dest_map_size > dest_bgrt->num_species) {
        // Create new (bigger) dest nodes array and copy old references.
        BgrTreeNode **new_nodes = (BgrTreeNode **) calloc(dest_map_size,
                sizeof(struct BgrTreeNode*));
        for (unsigned int i = 0; i < dest_bgrt->num_species; ++i)
            new_nodes[i] = dest_bgrt->nodes[i];

        // Copy & cleanup.
        free(dest_bgrt->nodes); // TODO: Wrong free!
        dest_bgrt->nodes = new_nodes;
        dest_bgrt->num_species = dest_map_size;
    }

    // Step 3: traverse the src BGRT and append all found signatures to the
    // dest BGRT.

    // Create a match array. Matched IDs are stored here during traversal.
    unsigned int match_array[dest_bgrt->num_species];
    std::stack<unsigned int> id_stack;
    std::stack<BgrTreeNode*> node_stack;
    BgrTreeNode *node = NULL;

    // Iterate through the root array (nodes)...
    for (unsigned int i = 0; i < src_bgrt->num_species; ++i) {
        node = src_bgrt->nodes[i];
        if (node) {
            // Push root node.
            node_stack.push(node);
            id_stack.push(0);

            // Iterate through all BGRT nodes below the current root node.
            while (!node_stack.empty()) {
                // Fetch last node and its number of matches from the stack.
                node = node_stack.top();
                node_stack.pop();
                unsigned int parents_matches = id_stack.top();
                id_stack.pop();

                if (node->species->size() > 0) { // TODO: UNNECESSARY IF...!?
                    // We have matched species entries in this node...
                    // WARNING: std::map operator[] may add unknown entries!
                    for (unsigned int j = 0; j < node->species->size(); ++j)
                        match_array[parents_matches + j] =
                                src2dest_mapping[node->species->val(j)];
                }
                unsigned int my_matches = parents_matches
                        + node->species->size();

                unsigned int size = 0;
                if (src_bgrt->base4_compressed)
                    size = node->signatures.base4->size();
                else
                    size = node->signatures.str->size();

                if (size > 0) {
                    // We have matched signatures in this node...
                    // Create an IntSet based on the current matches.
                    IntSet *set = new IntSet(my_matches);
                    for (unsigned int j = 0; j < my_matches; ++j)
                        set->add(match_array[j]); // TODO: UNTESTED!

                    //set->vals = (unsigned int*) malloc(
                    //my_matches * sizeof(unsigned int));
                    //memcpy(set->vals, match_array,
                    //my_matches * sizeof(unsigned int));
                    //set->size = set->vsize = my_matches;

                    // Add the signature and matching to the dest_bgrt.
                    if (src_bgrt->base4_compressed) {
                        for (unsigned int j = 0;
                                j < node->signatures.base4->size(); ++j) {
                            char *str = node->signatures.base4->val(j)->toChar(
                                    true);
                            BgrTree_insert(dest_bgrt, str, set,
                                    node->supposed_outgroup_matches->val(j));
                            // TODO: This is NOT the way it should work!
                            // Add a separate insert function for base4 sets.
                        }
                    } else
                        BgrTree_insert(dest_bgrt, node->signatures.str, set,
                                node->supposed_outgroup_matches);

                    // Increase copied nodes counter.
                    ++copied_nodes;
                }

                // Store reference to next node on this level.
                if (node->next) {
                    // Store parents number of matches onto the stack.
                    id_stack.push(parents_matches);
                    // Add next node(s) to stack.
                    node_stack.push(node->next);
                }
                // Store reference to child node(s). (Will be processed next.)
                if (node->children) {
                    // Store this nodes number of matches onto the stack.
                    id_stack.push(my_matches);
                    // Add child node(s) to stack.
                    node_stack.push(node->children);
                }

            }
        }
    }

    // Step 4: TODO: Merge BGRT parameters...
    mergeBGRTParams(dest_bgrt, src_bgrt);

    return copied_nodes;
}
