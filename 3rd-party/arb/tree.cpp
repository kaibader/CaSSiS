/*!
 * ARB phylogenetic tree loader for CaSSiS.
 *
 * Copyright (C) 2011,2012
 *     Kai Christian Bader <mail@kaibader.de>
 */

#include "tree.h"

#ifndef ARBDBT_H
#include <arbdbt.h>
#endif

#include <cassis/namemap.h>
#include <cassis/tree.h>

#include <iostream>

/*!
 * Create a CaSSiSTreeNode based on an ARB tree structure.
 * Helper function (recursion)
 */
CaSSiSTreeNode *fetchTreeFromARB_recursion(CaSSiSTree *tree,
        CaSSiSTreeNode *parent, GBT_TREE *gbt_tree, double length) {
    if (!gbt_tree)
        return NULL;

    // Create a new node
    CaSSiSTreeNode *node = new CaSSiSTreeNode(tree->allowed_outgroup_matches);

    // Give the node an index, starting with '0'.
    // Needed, if accessed through arrays...
    node->node_index = tree->num_nodes++;

    // Increase the node array size, if necessary.
    if (tree->num_nodes > tree->tour_index) {
        unsigned int new_size = tree->tour_index * 2;
        if (new_size == 0)
            new_size = 32; // Let's start with 32 entries...

        CaSSiSTreeNode **array = (CaSSiSTreeNode **) malloc(
                new_size * sizeof(CaSSiSTreeNode *));

        if (tree->internal_node_array) {
            memcpy(array, tree->internal_node_array,
                    tree->tour_index * sizeof(CaSSiSTreeNode *));
            free(tree->internal_node_array);
        }

        tree->internal_node_array = array;
        tree->tour_index = new_size;
    }

    // Add the node to the array.
    tree->internal_node_array[node->node_index] = node;

    // Set reference to parent, path and node depth
    node->parent = parent;
    if (parent)
        node->depth = parent->depth + 1;
    else
        node->depth = 0;

    // Set new max_depth, if required
    if (node->depth > tree->tree_depth)
        tree->tree_depth = node->depth;

    // Create a new IntSet to store the sequences within a subtree.
    node->group = new IntSet();

    // Add the branch length to the node.
    node->length = length;

    // Do we have a name which has to be stored in the node or the
    // matched species group?
    node->this_id = ID_TYPE_UNDEF;
    if (gbt_tree->name) {
        if (gbt_tree->is_leaf) {
            // Process a leaf node...
            node->this_id = node->leftmost_id = node->rightmost_id =
                    tree->leaf_mapping.append(gbt_tree->name);
            node->group->add(node->this_id);
        } else {
            // Just add the ID, if we are at an inner node
            node->this_id = tree->group_mapping.append(gbt_tree->name);
        }
    }

    // Traverse left and right child
    node->left = NULL;
    node->right = NULL;
    if (gbt_tree->leftson) {
        node->left = fetchTreeFromARB_recursion(tree, node, gbt_tree->leftson,
                gbt_tree->leftlen);
        IntSet *left_group = node->left->group;
        for (unsigned int i = 0; i < left_group->size(); i++)
            node->group->add(left_group->val(i));
        node->leftmost_id = node->left->leftmost_id;
    }
    if (gbt_tree->rightson) {
        node->right = fetchTreeFromARB_recursion(tree, node, gbt_tree->rightson,
                gbt_tree->rightlen);
        IntSet *right_group = node->right->group;
        for (unsigned int i = 0; i < right_group->size(); i++)
            node->group->add(right_group->val(i));
        node->rightmost_id = node->right->rightmost_id;
    }
    return node;
}

/*!
 * Create a CaSSiSTree based on an ARB database and tree structure.
 *
 * \param arb_db_name Name of the ARB database file
 * \param arb_tree Name of the ARB tree to be fetched/parsed
 * \param allowed_og_matches Number of allowed outgroup matches
 * \return CaSSiSTree, if successful. Otherwise NULL.
 */
CaSSiSTree *fetchTreeFromARB(const char *arb_db_name, const char *arb_tree,
        unsigned int allowed_og_matches) {
    // Define a dummy ARBHOME environment variable
    setenv("ARBHOME", ".", 1);

    // Create gb_shell object...
    GB_shell *gb_shell = new GB_shell;

    // Open the ARB database
    GBDATA *gb_db = GBT_open(arb_db_name, "rt");
    if (!gb_db) {
        fprintf(stderr, "Error while opening the ARB DB: %s\n", arb_db_name);
        return NULL;
    }

    // Fetch the ARB tree...
    printf("Reading tree \"%s\"... ", arb_tree);
    GB_begin_transaction(gb_db);
    GBT_TREE *gbt_tree = GBT_read_tree(gb_db, arb_tree, sizeof(GBT_TREE));

    if (!gbt_tree) {
        fprintf(stderr, "Phylogenetic tree \"%s\" not found!\n", arb_tree);
        GB_close(gb_db);
        return NULL;
    }

    printf("done.\n");
    GBT_link_tree(gbt_tree, gb_db, false, 0, 0);
    GB_commit_transaction(gb_db);

    // Create a new CaSSiSTree...
    CaSSiSTree *tree = new CaSSiSTree();

    // Create the nodes of the phylogenetic tree
    if (tree) {
        // WARNING: This is a dirty 'hack'!
        // The size of the trees 'node_array' is stored in here
        // during the traversal of the ARB tree.
        tree->tour_index = 0;

        tree->allowed_outgroup_matches = allowed_og_matches;
        tree->tree_root = fetchTreeFromARB_recursion(tree, NULL, gbt_tree, 0.0);

        tree->tour_index = 0;
    }

    if (!tree || !tree->tree_root) {
        fprintf(stderr, "Unable to read/create the "
                "CaSSiSTree structure.\n");
    }

    // Close the ARB database and return the phylogenetic tree...
    GBT_delete_tree(gbt_tree);
    GB_close(gb_db);
    delete gb_shell;
    return tree;
}
