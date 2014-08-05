/*!
 * Newick formatted tree file reader
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
 *
 * Copyright (C) 2010-2012
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

#include "newick.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <stack>

/*!
 * Size of the read buffer
 */
const unsigned int NEWICK_READ_BUFFER_SIZE = 1024 * 64;

void DumpCaSSiSTreeNode(const CaSSiSTreeNode *node, NameMap &map) {
    if (!node)
        return;

    if (node->left && node->right) {
        printf("(");
        DumpCaSSiSTreeNode(node->left, map);
        printf(",");
        DumpCaSSiSTreeNode(node->right, map);

        printf(")");

        if (node->this_id != ID_TYPE_UNDEF)
            printf("\'%s\'\n", map.name(node->this_id).c_str());
    } else if (node->this_id != ID_TYPE_UNDEF) {
        printf("\'%s\'\n", map.name(node->this_id).c_str());
    }
}

/*!
 * Dump a phylogenetic tree in the Newick format to stdout.
 *
 * \param tree Phylogenetic tree
 */
void DumpCaSSiSTree(const CaSSiSTree *tree, NameMap &map) {
    if (!tree)
        return;
    DumpCaSSiSTreeNode(tree->tree_root, map);
    printf(";\n");
}

/*!
 * Post-process the phylogenetic tree. Add statistical information.
 * \param tree Phylogenetic tree
 */
void Newick2CaSSiSTree_postprocess(const NameMap &map, CaSSiSTree *tree,
        CaSSiSTreeNode *node, unsigned int &node_array_pos, unsigned int depth =
                0) {
    if (!node)
        return; // No check for 'tree'. It is assumed that there is one...

    // Set the node depth. Update the max. tree depth if necessary.
    node->depth = depth;
    if (depth > tree->tree_depth)
        tree->tree_depth = depth;

    // Add a reference to the node to the internal node array.
    node->node_index = node_array_pos;
    tree->internal_node_array[node_array_pos++] = node;

    // Create a new IntSet to store the sequences within a subtree.
    node->group = new IntSet();

    if (node->left && node->right) {
        // We are processing an inner (group) node...
        std::string name = map.name(node->this_id);
        if (name.length() > 0)
            node->this_id = tree->group_mapping.append(name);
        else
            node->this_id = ID_TYPE_UNDEF;

        // Pre-processing (Euler Tour)
        tree->eulertour[tree->tour_index] = node;
        tree->level[tree->tour_index++] = tree->tour_level++;

        // Create left node and link it to the current node.
        Newick2CaSSiSTree_postprocess(map, tree, node->left, node_array_pos,
                depth + 1);
        node->leftmost_id = node->left->leftmost_id;

        IntSet *left_group = node->left->group;
        for (unsigned int i = 0; i < left_group->size(); i++)
            node->group->add(left_group->val(i));

        // In-processing (Euler Tour)
        tree->eulertour[tree->tour_index] = node;
        tree->level[tree->tour_index++] = tree->tour_level - 1;

        // Create right node and link it to the current node.
        Newick2CaSSiSTree_postprocess(map, tree, node->right, node_array_pos,
                depth + 1);
        node->rightmost_id = node->right->rightmost_id;

        // Post-processing (Euler Tour)
        tree->eulertour[tree->tour_index] = node;
        tree->level[tree->tour_index++] = --tree->tour_level;

        IntSet *right_group = node->right->group;
        for (unsigned int i = 0; i < right_group->size(); i++)
            node->group->add(right_group->val(i));

    } else if (!node->left && !node->right) {
        // We are processing a leaf node. Fetch a new leaf node id from our
        // mapping. The mapping starts with ID '0' and is also used as index
        // position in the representative array.
        assert(node->this_id != ID_TYPE_UNDEF);

        // Take the temporary ID and replace it with a new leaf-ID.
        node->this_id = node->leftmost_id = node->rightmost_id =
                tree->leaf_mapping.append(map.name(node->this_id));

        // Pre-processing (Euler Tour)
        tree->eulertour[tree->tour_index] = node;
        tree->level[tree->tour_index] = tree->tour_level;
        tree->representative[node->this_id] = tree->tour_index++;

        node->group->add(node->this_id); // TODO Needed for BGRT traversal?
    } else {
        // There is an error in the tree! Should not occur!
        assert(false);
    }
}

/*!
 * Create/read a CaSSiS tree from a Newick tree file.
 *
 * \param filename String containing the name of the Newick file.
 * \return CaSSiSTree, if successful. Otherwise NULL.
 */
CaSSiSTree *Newick2CaSSiSTree(const char *filename, unsigned int og_limit) {
    if (!filename) {
        fprintf(stderr, "Error: Empty Newick tree name.\n");
        return NULL;
    }

    // Create a new CaSSiS tree.
    CaSSiSTree* tree = new CaSSiSTree();
    if (!tree) {
        fprintf(stderr,
                "Error: Unable to allocate memory for the phylogenetic tree.\n");
        return NULL;
    }

    // Create a temporary name mapping.
    NameMap tmp_map;

    // Open the Newick file with read access
    FILE *fd = NULL;
    if ((fd = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Error while opening Newick file: %s\n", filename);
        return NULL;
    }

    // Create a read buffer
    char *read_buffer = (char *) malloc(NEWICK_READ_BUFFER_SIZE * sizeof(char));
    if (read_buffer == NULL) {
        fprintf(stderr, "Error while creating the read buffer.\n");
        return NULL;
    }

    // Create a string buffer (used to parse node names and distances)
    char *id_buffer = (char *) malloc(NEWICK_READ_BUFFER_SIZE * sizeof(char));
    if (id_buffer == NULL) {
        fprintf(stderr, "Error while creating the id buffer.\n");
        return NULL;
    }
    id_buffer[0] = 0;
    unsigned int id_buffer_size = 0; // Counter: number of chars in the id_buffer.

    // This flag is set to true during parsing if we are within a comment.
    bool comment = false;

    // This flag is set to true during parsing if we are within a string.
    int exclamation = 0;

    // Some pointers I need later...
    CaSSiSTreeNode *node1, *node2, *node3;

    // Set a generic locale (avoids float/double errors)
    const char* oldLC_const = setlocale(LC_NUMERIC, 0);
    char *oldLC = (char*) malloc(strlen(oldLC_const) + 1);
    strcpy(oldLC, oldLC_const);
    setlocale(LC_NUMERIC, "C");

    // Create a PhyloTreeNode stack structure and push a first node onto it.
    std::stack<CaSSiSTreeNode*> node_stack;
    node1 = new CaSSiSTreeNode(og_limit);
    node_stack.push(node1);
    tree->num_nodes++;

    while (true) {
        // Read a line of the file
        char *ret = fgets(read_buffer, NEWICK_READ_BUFFER_SIZE - 1, fd);

        // Abort reading if we are at the end of the file.
        if ((ret == NULL) || feof(fd) || ferror(fd))
            break;

        // Just to be safe...
        read_buffer[NEWICK_READ_BUFFER_SIZE - 1] = 0;

        // The parsing happens here...
        char *it = read_buffer;
        char *len_str = NULL;
        while (*it) {
            // There always has to be at least one node on the stack...
            assert(node_stack.size());
            if (node_stack.size() == 0) {
                fprintf(stderr,
                        "Error: Apparently not a valid binary Newick tree?\n");
                return NULL;
            }

            if (!comment) {
                switch (*it) {
                case '[':
                    comment = true;
                    break;
                case ',':
                    // Break here, if we are within a text area
                    if (exclamation)
                        break;

                    // Fetch last node on stack
                    node1 = node_stack.top();

                    // Fetch length, if available and clean-up buffer string
                    len_str = NULL;
                    if ((len_str = strchr(id_buffer, ':')) != NULL) {
                        *len_str = 0;
                        ++len_str;
                    }

                    // Add node id...
                    if (*id_buffer)
                        node1->this_id = tmp_map.append(id_buffer);

                    // Add branch length, if available...
                    if (len_str)
                        node1->length = atof(len_str);

                    // Reset buffer...
                    id_buffer_size = 0;
                    id_buffer[0] = 0;

                    // Create a second node and add it to the stack
                    node2 = new CaSSiSTreeNode(og_limit);
                    node_stack.push(node2);
                    tree->num_nodes++;
                    break;
                case ')':
                    // Break here, if we are within a text area
                    if (exclamation)
                        break;

                    // Closing bracket -> min. one element within bracket.
                    assert(node_stack.size() >= 2);
                    if (node_stack.size() <= 1) {
                        fprintf(stderr,
                                "Error: Apparently not a valid binary Newick tree?\n");
                        return NULL;
                    }

                    // Fetch/pop two nodes from stack
                    node1 = node_stack.top();
                    node_stack.pop();
                    node2 = node_stack.top();
                    node_stack.pop();

                    // Create a third node as a parent for the other two
                    node3 = new CaSSiSTreeNode(og_limit);

                    node3->left = node2;
                    node2->parent = node3;

                    node3->right = node1;
                    node1->parent = node3;

                    tree->num_nodes++;

                    // Fetch length, if available and clean-up buffer string
                    len_str = NULL;
                    if ((len_str = strchr(id_buffer, ':')) != NULL) {
                        *len_str = 0;
                        ++len_str;
                    }

                    // Add node id...
                    if (*id_buffer)
                        node1->this_id = tmp_map.append(id_buffer);

                    // Add branch length, if available...
                    if (len_str)
                        node1->length = atof(len_str);

                    // Reset buffer...
                    id_buffer_size = 0;
                    id_buffer[0] = 0;

                    // Push the parent node onto the stack
                    node_stack.push(node3);
                    break;
                    // case ' ':
                case '\'':
                    if (exclamation == 0) {
                        exclamation = 1;
                        break;
                    } else if (exclamation == 1) {
                        exclamation = 0;
                        break;
                    } else if (id_buffer_size == NEWICK_READ_BUFFER_SIZE)
                        break;
                    id_buffer[id_buffer_size++] = *it;
                    id_buffer[id_buffer_size] = 0;
                    break;
                case '\"':
                    if (exclamation == 0) {
                        exclamation = 2;
                        break;
                    } else if (exclamation == 2) {
                        exclamation = 0;
                        break;
                    } else if (id_buffer_size == NEWICK_READ_BUFFER_SIZE)
                        break;
                    id_buffer[id_buffer_size++] = *it;
                    id_buffer[id_buffer_size] = 0;
                    break;
                case '\t':
                case '\r':
                case '\n':
                case '(':
                    // Ignore these characters...
                    break;
                default:
                    if (id_buffer_size == NEWICK_READ_BUFFER_SIZE)
                        break;
                    id_buffer[id_buffer_size++] = *it;
                    id_buffer[id_buffer_size] = 0;
                    break;
                }
            } else if (*it == ']')
                comment = false;

            it++; // Next character.
        }
    }

    // One node remains as root node when the structure was finally evaluated...
    assert(node_stack.size() == 1);
    if (node_stack.size() != 1) {
        fprintf(stderr, "Error: Apparently not a valid binary Newick tree?\n");
        return NULL;
    }

    // Set root node of phylogenetic tree and return it.
    CaSSiSTreeNode *root_node = node_stack.top();
    tree->setRootNode(root_node);

    // Clean-up buffer string
    if (char *s = strchr(id_buffer, ':'))
        *s = 0;

    // Add node id...
    if (*id_buffer && *id_buffer != ';') {
        if (char *s = strchr(id_buffer, ';'))
            *s = 0;
        root_node->this_id = tmp_map.append(id_buffer);
    }

    // Restore old locale
    setlocale(LC_NUMERIC, oldLC);
    free(oldLC);

    // Set the trees' outgroup limit.
    tree->allowed_outgroup_matches = og_limit;

    // Initialize the lookup-arrays that allow us constant-time LCA-searches!
    // See: Bender, Michael A., Farach-Colton, Martin (2000), "The LCA problem
    // revisited", Proceedings of the 4th Latin American Symposium on
    // Theoretical Informatics, Lecture Notes in Computer Science, 1776,
    // Springer-Verlag, pp. 88–94, doi:10.1007/10719839_9

    // The size of our lookup-arrays depends on the number of tree nodes.
    tree->tour_size = 2 * tree->num_nodes - 1;
    tree->log_tour_size = tree->binLog(tree->tour_size) + 1;
    // TODO: Here +1 necessary for the '0' index?

    // Clean-up...
    free(tree->eulertour);
    tree->eulertour = (CaSSiSTreeNode**) malloc(
            tree->tour_size * sizeof(CaSSiSTreeNode*));

    // Clean-up level...
    tree->level = (unsigned int *) malloc(
            tree->tour_size * sizeof(unsigned int));

    // Clean-up representative...
    tree->representative = (unsigned int *) malloc(
            tree->tour_size * sizeof(unsigned int));
    // TODO: Larger than required. "phylo_tree->num_nodes" should be enough!

    // Clean-up sparse_table...
    tree->sparse_table = (unsigned int **) malloc(
            tree->tour_size * sizeof(unsigned int *));
    for (unsigned int i = 0; i < tree->tour_size; ++i)
        tree->sparse_table[i] = (unsigned int *) malloc(
                tree->log_tour_size * sizeof(unsigned int));

    // Reset position counter...
    tree->tour_index = 0;
    tree->tour_level = 0;

    tree->internal_node_array = (CaSSiSTreeNode**) malloc(
            tree->num_nodes * sizeof(CaSSiSTreeNode*));

    // Post-process the phylogenetic tree (i.e. all nodes)
    unsigned int node_array_pos = 0;
    Newick2CaSSiSTree_postprocess(tmp_map, tree, tree->getRootNode(),
            node_array_pos);

    // Post-process the sparse table
    tree->computeSparseTable();

    // Cleaning up after all work is done...
    node_stack.empty();
    free(read_buffer);
    free(id_buffer);
    fclose(fd);
    return tree;
}
