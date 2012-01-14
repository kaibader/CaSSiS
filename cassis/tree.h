/*!
 * Unified CaSSiS Tree structure. Stores a phylogenetic tree.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
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

#ifndef CASSIS_TREE_H_
#define CASSIS_TREE_H_

#include "types.h"
#include "namemap.h"

/*!
 * CaSSiS Tree node.
 */
class CaSSiSTreeNode {
public:
    CaSSiSTreeNode(unsigned int allowed_outgroup_matches);
    ~CaSSiSTreeNode();
    bool isLeaf();

    /*!
     * This function is adds a signature <-> sequence relationship
     * to the node. (Replaces an existing matching, if better.)
     * \return true, if successfully added. Otherwise false.
     */
    bool addMatching(char *signature, unsigned int ingroup_matches,
            unsigned int outgroup_matches);

    /*!
     * Reference to the parent and child nodes.
     */
    CaSSiSTreeNode *left;
    CaSSiSTreeNode *right;
    CaSSiSTreeNode *parent;

    /*!
     * This nodes', leftmost and rightmost identifiers.
     * Leaf nodes: leftmost_id == rightmost_id
     */
    id_type this_id;
    id_type leftmost_id;
    id_type rightmost_id;

    /*!
     * The following three arrays store the best matches according the number
     * of ingroup matches.
     */
    unsigned int *num_matches;
    StrRefSet *signatures;

    /*!
     * Depth of the node within the tree
     */
    unsigned int depth;

    /*!
     * Distance between the node and its parent node.
     * This value is not used by CaSSiS and could be removed if necessary.
     */
    double length;

    /*!
     * To be able to sort the nodes within an array, every node
     * receives an unique index position. This is not the ID!
     */
    unsigned int node_index;

    /*!
     * Set of species IDs, which are covered by the group node
     * (All leaf IDs below this inner node.)
     */
    IntSet *group;

    /*!
     * Starting solution (an optimization for BGRT processing).
     */
    unsigned int starting_solution;

    /*!
     * Highest ingroup coverage archieved by a signature within
     * the outgroup range.
     */
    unsigned int best_ingroup_coverage;
};

class CaSSiSTree {
public:
    /*!
     * Constructor and Destructor
     */
    CaSSiSTree();
    virtual ~CaSSiSTree();

    /*!
     * This function is adds a signature-sequence relationship
     * to the search tree.
     */
    bool addMatching(const char *signature, IntSet *matches,
            unsigned int unspecified_outgroup_matches);

    /*!
     * Fetch a NameMap that fits to the CaSSiSTree entries.
     */
    void fetchMapping(NameMap &map);

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
    bool enforceExtMapping_internal(const NameMap &map, CaSSiSTreeNode *node);
    bool enforceExtMapping(const NameMap &map);

    /*!
     * This function sets a cassis_node as root node.
     */
    void setRootNode(CaSSiSTreeNode *cassis_node);

    /*!
     * This function returns the root node, if available
     */
    CaSSiSTreeNode *getRootNode();

    /*!
     * Computes the binary logarithm of an unsigned 32 bit value.
     * Returns 0 when 'bits' equals 0x00000000.
     */
    unsigned int binLog(unsigned int bits);

    /*!
     * Build the sparse table (divide and conquer)
     */
    void computeSparseTable();

    /*!
     * Range Minimum Query
     * \param i Lowest index position in the level array L.
     * \param j Highest index position in the level array L.
     * \return Index position with the smallest value in the range L[i,j].
     */
    unsigned int RMQ(unsigned int i, unsigned int j);

    /*!
     * This method is used propagate a matching (downwards) within a subtree.
     *
     * \param node The CaSSiS node that should be processed.
     * \param signature The matching signature
     * \param matches IDs of the matched sequences
     * \param unspecified_outgroup_matches Number of outgroup matches that are not
     *        directly related to the tree structure.
     */
    void propagateDownwards(CaSSiSTreeNode *node, char *signature,
            IntSet *matches, unsigned int unspecified_outgroup_matches);

public:
    unsigned int allowed_outgroup_matches;
    CaSSiSTreeNode *tree_root;
    StrSet signatures;
    NameMap leaf_mapping;
    NameMap group_mapping;
    bool uses_external_mapping;
    //
    size_t tour_size;
    size_t log_tour_size;
    CaSSiSTreeNode **eulertour;
    unsigned int *level;
    unsigned int *representative;
    unsigned int **sparse_table;
    unsigned int tour_index;
    unsigned int tour_level;

    // TODO: These member variables are public as long as the refactoring
    // goes on...
    unsigned int num_nodes;
    unsigned int tree_depth;
    CaSSiSTreeNode **internal_node_array;
};

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
void reduceCaSSiSTreeDepth(CaSSiSTree *tree);

#endif /* CASSIS_TREE_H_ */
