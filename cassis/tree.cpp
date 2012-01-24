/*!
 * Unified CaSSiS Tree structure. Stores a phylogenetic tree.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
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

#include "tree.h"

#include <cassert>
#include <cstring>

/*!
 * CaSSiSTreeNode constructor
 */
CaSSiSTreeNode::CaSSiSTreeNode(unsigned int allowed_outgroup_matches) :
left(NULL), right(NULL), parent(NULL), this_id(ID_TYPE_UNDEF), leftmost_id(
        ID_TYPE_UNDEF), rightmost_id(ID_TYPE_UNDEF), num_matches(NULL), signatures(
                NULL), group(NULL), starting_solution((unsigned int) -1), best_ingroup_coverage(
                        0) {
    signatures = new StrRefSet[allowed_outgroup_matches + 1];
    num_matches = new unsigned int[allowed_outgroup_matches + 1];

    for (unsigned int i = 0; i <= allowed_outgroup_matches; ++i)
        num_matches[i] = 0;

    leftmost_id = rightmost_id = this_id = ID_TYPE_UNDEF;
}

/*!
 * CaSSiSTreeNode destructor.
 * Recursively cleans a subtree of CaSSiSTreeNodes.
 */
CaSSiSTreeNode::~CaSSiSTreeNode() {
    delete left;
    delete right;
    delete[] num_matches;
    delete[] signatures;
    //
    delete group;
}

/*!
 * Test, if a node is a leaf
 */
bool CaSSiSTreeNode::isLeaf() {
    return (!left && !right);
}

/*!
 * This function is adds a signature-sequence relationship
 * to the node. (Replaces an existing matching, if better.)
 */
bool CaSSiSTreeNode::addMatching(char *signature, unsigned int ingroup_matches,
        unsigned int outgroup_matches) {
    if ((ingroup_matches > 0)
            && (ingroup_matches >= num_matches[outgroup_matches])) {
        if (ingroup_matches > num_matches[outgroup_matches]) {
            // Create a new signature list, as we have a better solution.
            signatures[outgroup_matches].clear();
            num_matches[outgroup_matches] = ingroup_matches;
        }
        // Add our signature to the list.
        signatures[outgroup_matches].add(signature);

        // Store the best coverage 'score' that was achieved.
        if (ingroup_matches > best_ingroup_coverage)
            best_ingroup_coverage = ingroup_matches;

        // Success...
        return true;
    }
    return false;
}

/*!
 * Computes the binary logarithm of an unsigned 32 bit value.
 * Returns 0 when 'bits' equals 0x00000000.
 */
unsigned int CaSSiSTree::binLog(unsigned int bits) {
    int log = 0;
    if ((bits & 0xffff0000) != 0) {
        bits = bits >> 16;
        log = 16;
    }
    if (bits >= 256) {
        bits = bits >> 8;
        log += 8;
    }
    if (bits >= 16) {
        bits = bits >> 4;
        log += 4;
    }
    if (bits >= 4) {
        bits = bits >> 2;
        log += 2;
    }
    return log + (bits >> 1);
}

/*!
 * CaSSiSTree -- Constructor.
 */
CaSSiSTree::CaSSiSTree() :
                allowed_outgroup_matches(0), tree_root(NULL), uses_external_mapping(
                        false), tour_size(0), log_tour_size(0), eulertour(NULL), level(
                                NULL), representative(NULL), sparse_table(NULL), tour_index(0), tour_level(
                                        0), num_nodes(0), tree_depth(0), internal_node_array(NULL) {

}

/*!
 * CaSSiSTree -- Destructor.
 */
CaSSiSTree::~CaSSiSTree() {
    delete tree_root;
    free(eulertour);
    free(level);
    free(representative);
    for (unsigned int i = 0; i < tour_size; ++i)
        free(sparse_table[i]);
    free(sparse_table);
    free(internal_node_array);
}

/*!
 * This function sets a cassis_node as root node.
 */
void CaSSiSTree::setRootNode(CaSSiSTreeNode *cassis_node) {
    if (cassis_node)
        tree_root = cassis_node;
}

/*!
 * This function returns the root node, if available
 */
CaSSiSTreeNode *CaSSiSTree::getRootNode() {
    return tree_root;
}

/*!
 * This function is adds a signature-sequence relationship
 * to the search tree.
 */
bool CaSSiSTree::addMatching(const char *signature, IntSet *matches,
        unsigned int unspecified_outgroup_matches) {
    // This function does not work in combination with external mappings.
    if (uses_external_mapping)
        return false;

    // Ignore signatures/matches that already have more than the allowed
    // number of outgroup matches.
    if (unspecified_outgroup_matches > allowed_outgroup_matches)
        return false;

    // (1) Fetch the lowest and highest leaf IDs.
    // (2) Fetch the index position of their first occurrence from the
    //     representative array.
    // (3) Fetch the RMQ between these two positions from the level array.
    unsigned int num_matches = matches->size();
    if (num_matches > 0) {
        unsigned int pos = RMQ(representative[matches->val(0)],
                representative[matches->val(num_matches - 1)]);

        // Add a copy of the signature to our internal list of signatures.
        // Only pointers to them are stored in the CaSSiS Tree nodes.
        char *sig_ptr = strdup(signature);
        signatures.add(sig_ptr);

        // (4) The node at eulertour[pos] represents the LCA of the given leaves.
        CaSSiSTreeNode *node = eulertour[pos];

        // Propagate the matching downwards, starting with the found node.
        propagateDownwards(node, sig_ptr, matches,
                unspecified_outgroup_matches);

        // Propagate the matching upwards, as long as we have found an equal
        // or better result.
        node = node->parent;
        while (node
                && (num_matches
                        >= node->num_matches[unspecified_outgroup_matches])) {
            if (num_matches > node->num_matches[unspecified_outgroup_matches]) {
                node->signatures[unspecified_outgroup_matches].clear();
                node->num_matches[unspecified_outgroup_matches] = num_matches;
            }
            node->signatures[unspecified_outgroup_matches].add(sig_ptr);

            // Go one level up in the tree.
            node = node->parent;
        }
    }
    return true;
}

/*!
 * This method is used propagate a matching (downwards) within a subtree.
 *
 * Comments:
 * - The first node that will be processed is the LCA with the best coverage.
 * - Child nodes will have less coverage, but the signature may still be
 *   relevant if outgroup matches are allowed.
 * - Leaf IDs are sorted in ascending order (left to right), without gaps.
 *   The matches (IntSet*) are also sorted in ascending order.
 *   --> We can determine the number of outgroup matches by comparing the
 *   smallest and highest IDs from both ranges.
 *
 * \param node The CaSSiS node that should be processed.
 * \param signature The matching signature
 * \param matches IDs of the matched sequences
 * \param unspecified_outgroup_matches Number of outgroup matches that are not
 *        directly related to the tree structure.
 */
void CaSSiSTree::propagateDownwards(CaSSiSTreeNode *node, char *signature,
        IntSet *matches, unsigned int unspecified_outgroup_matches) {
    // Find out how many outgroup matches we have including this node.
    // Check for leaves==matches left of the current subtree.
    unsigned int outgroup_matches_here = unspecified_outgroup_matches;
    unsigned int matches_size = matches->size();
    unsigned int pos = 0;
    while ((pos < matches_size) && (node->leftmost_id > matches->val(pos)))
        ++pos;
    outgroup_matches_here += pos;

    // Ignore signatures that produce more than the allowed
    // number of outgroup matches. Do not further propagate.
    if (outgroup_matches_here > allowed_outgroup_matches)
        return;

    // Check for leaves==matches right of the current subtree.
    // Comment: (pos < matches_size) check might be dirty, but works.
    pos = matches_size - 1;
    while ((pos < matches_size) && (node->rightmost_id < matches->val(pos)))
        --pos;
    outgroup_matches_here += matches_size - pos - 1;

    // Ignore signatures that produce more than the allowed
    // number of outgroup matches. Do not further propagate.
    if (outgroup_matches_here > allowed_outgroup_matches)
        return;

    // Compute the number of matches for this(!) node.
    unsigned int num_matches = matches_size
            - (outgroup_matches_here - unspecified_outgroup_matches);

    if (num_matches) {
        if (num_matches >= node->num_matches[outgroup_matches_here]) {
            if (num_matches > node->num_matches[outgroup_matches_here]) {
                node->signatures[outgroup_matches_here].clear();
                node->num_matches[outgroup_matches_here] = num_matches;
            }
            node->signatures[outgroup_matches_here].add(signature);
        }

        // Propagate the matching to the child nodes, if available.
        if (node->left)
            propagateDownwards(node->left, signature, matches,
                    unspecified_outgroup_matches);
        if (node->right)
            propagateDownwards(node->right, signature, matches,
                    unspecified_outgroup_matches);
    }
}

/*!
 * Initialize a NameMap with IDs the fit to the CaSSiSTree entries.
 * \param map NameMap that should be initialized. Old content will be deleted.
 */
void CaSSiSTree::fetchMapping(NameMap &map) {
    map.clear();
    map.setSize(leaf_mapping.size() + group_mapping.size());

    if (uses_external_mapping) {
        map = leaf_mapping;
    } else {
        for (unsigned int i = 0; i < leaf_mapping.size(); ++i)
            map.set(i, leaf_mapping.name(i));
        for (unsigned int i = 0; i < group_mapping.size(); ++i)
            map.set(leaf_mapping.size() + i, group_mapping.name(i));
    }
}

bool CaSSiSTree::enforceExtMapping_internal(const NameMap &map,
        CaSSiSTreeNode *node) {
    assert(node != NULL);
    bool retval = true;

    if (node->isLeaf()) {
        if (node->this_id != ID_TYPE_UNDEF) {
            id_type id = map.id(leaf_mapping.name(node->this_id));

            // Set return value to false if the name has no corresponding
            // ID in the new mapping.
            if (id == ID_TYPE_UNDEF)
                retval = false;

            // Set the nodes' ID.
            node->group->clear();
            node->group->add(id);
            node->this_id = id;
        }
    } else {
        // Clear the group ID list...
        node->group->clear();

        // Process the left subtree...
        retval = enforceExtMapping_internal(map, node->left);

        // Fetch NEW group IDs from the left child.
        IntSet *group = node->left->group;
        for (unsigned int i = 0; i < group->size(); i++)
            node->group->add(group->val(i));

        // Process the right subtree...
        retval &= enforceExtMapping_internal(map, node->right);

        // Fetch NEW group IDs from the right child.
        group = node->right->group;
        for (unsigned int i = 0; i < group->size(); i++)
            node->group->add(group->val(i));

    }
    return retval;
}

/*
 * Enforces an external mapping.
 * Leaf IDs are overwritten (or set to ID_TYPE_UNDEF if unreferenced).
 * Group IDs are kept as defined by the tree!
 * Comment: '1pass' tree processing is disabled by this feature.
 *
 * \param map Mapping that should be used for the tree.
 * \return true, if all nodes were successfully remapped.
 * References that could not be remapped will be set to ID_TYPE_UNDEF.
 */
bool CaSSiSTree::enforceExtMapping(const NameMap &map) {
    // From this moment on we will switch to an external mapping scheme.
    uses_external_mapping = true;

    // Recursively propagate the new IDs...
    bool retval = enforceExtMapping_internal(map, tree_root);

    // Finally, clear the internal mapping.
    // The external 'map' will be copied to our 'leaf_mapping'.
    leaf_mapping = map;
    return retval;
}

/*!
 * Build the sparse table (divide and conquer)
 */
void CaSSiSTree::computeSparseTable() {

    // Compute entries with a range of '1'...
    for (unsigned int i = 0; i < tour_size; ++i)
        sparse_table[i][0] = i;

    // Compute entries for the higher ranges...
    for (unsigned int j = 1; (1 << j) <= tour_size; ++j)
        for (unsigned int i = 0; i + (1 << j) - 1 < tour_size; ++i)
            if (level[sparse_table[i][j - 1]]
                      < level[sparse_table[i + (1 << (j - 1))][j - 1]])
                sparse_table[i][j] = sparse_table[i][j - 1];
            else
                sparse_table[i][j] = sparse_table[i + (1 << (j - 1))][j - 1];
}

/*!
 * Range Minimum Query
 * \param i Lowest index position in the level array L.
 * \param j Highest index position in the level array L.
 * \return Index position with the smallest value in the range L[i,j].
 */
unsigned int CaSSiSTree::RMQ(unsigned int i, unsigned int j) {
    assert(i <= j);
    unsigned int k = binLog(j - i + 1);

    unsigned int pos1 = sparse_table[i][k];
    unsigned int pos2 = sparse_table[j - (1 << k) + 1][k];

    return (level[pos1] <= level[pos2]) ? pos1 : pos2;
}

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/*!
 * Internal function for the rebalancing of a phylogenetic tree.
 *
 * - If a node has one leaf as child node, make this leaf to the left child.
 *   (The left child node is traversed first.)
 * - Do not increase the depth counter for nodes with a left child.
 *
 * \param node Current phylogenetic tree node
 * \return Maximum depth of the current subtree
 */
unsigned int reduceCaSSiSTreeDepth_recursion(CaSSiSTreeNode *node) {
    unsigned int depth = node->depth;

    // Return current depth, if we are a leaf.
    if (node->isLeaf())
        return depth;

    // Swap child node positions, if the right child is a leaf.
    if (node->right && node->right->isLeaf()) {
        CaSSiSTreeNode *tmp_node = node->left;
        node->left = node->right;
        node->right = tmp_node;
    }

    // If left child node is a leaf, set the child node depth to our depth.
    if (node->left && node->left->isLeaf()) {
        node->left->depth = depth;
        if (node->right)
            node->right->depth = depth;
    } else {
        if (node->left)
            node->left->depth = depth + 1;
        if (node->right)
            node->right->depth = depth + 1;
    }

    // Enter left child...
    if (node->left) {
        unsigned int left_depth = reduceCaSSiSTreeDepth_recursion(node->left);
        depth = MAX(depth, left_depth);
    }

    // Enter right child...
    if (node->right) {
        unsigned int right_depth = reduceCaSSiSTreeDepth_recursion(node->right);
        depth = MAX(depth, right_depth);
    }

    return depth;
}

/*!
 * This function rebalances the phylogenetic tree and modifies the node
 * depths to reduce the memory consumption of the BGRTree during traversal.
 *
 * - If a node has one leaf as child node, make this leaf to the left child.
 *   (The left child node is traversed first.)
 * - Do not increase the depth counter for nodes with a left child.
 *
 * \param tree Phylogenetic tree.
 */
void reduceCaSSiSTreeDepth(CaSSiSTree *tree) {
    // Recursive depth reduction...
    unsigned int new_depth = reduceCaSSiSTreeDepth_recursion(tree->tree_root);

    // Store new depth in the tree.
    tree->tree_depth = new_depth;
}

