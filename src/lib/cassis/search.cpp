/*!
 * BGRT search/traversal functionalities
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
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

#include "search.h"
#include "config.h"

#ifdef PTHREADS
#include "pool.h"
#include <pthread.h>
static unsigned int static_num_processors = 0;
void setNumProcessors(unsigned int n) {
    static_num_processors = n;
}
#endif

#include <cstdlib>
#include <cassert>
#include <climits>

/*!
 * Output verbose debug information during the tree traversal.
 * Disabled by default.
 */
#define DEBUG_PRINTFS 0

#if DEBUG_PRINTFS
#include <cstdio>
#endif

/*!
 * Flag: Enables/disables a progress-meter for the tree traversal part.
 * Disabled by default.
 */
#define PROGRESS_METER 1

#if PROGRESS_METER
#include <cstdio>
#endif

/*!
 * This flag enables/disables the cut-off mechanism during the signature
 * search within a BGRT. Enabled by default.
 */
#define USE_CUTTABLES 1

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

/*!
 * Traverse a BgrTree and evaluate the one node of the PhyloTree.
 * -- Recursion.
 */
void traverse_BgrTree_recursion(unsigned int starting_solution,
        struct BgrTreeNode *bgr_node, CaSSiSTreeNode *cassis_node,
        unsigned int cassistree_depth, unsigned int max_outgroup_hits,
        unsigned int *cutoff_array, unsigned int parent_ingroup_counter,
        unsigned int parent_outgroup_counter, bool base4_compression) {
    unsigned int phy_node_depth = cassis_node->depth;

    // Return without results, if the node or group is undefined
    if (bgr_node == NULL)
        return;

#ifdef USE_CUTTABLES
    if (!bgr_node->ingroup_array) {
        // Allocate memory for an ingroup array, if necessary.
        bgr_node->ingroup_array = (unsigned int *) calloc(cassistree_depth,
                sizeof(unsigned int));
        assert(phy_node_depth == 0);
    }
    assert(phy_node_depth <= cassistree_depth);

    // Create a reference to the ingroup array (may speed up r/w).
    unsigned int *node_ingroup_array = bgr_node->ingroup_array;

    // Update (increase) the node ingroup array...
    for (unsigned int i = 0; i < phy_node_depth; ++i)
        if (node_ingroup_array[i] == ID_TYPE_UNDEF )
            ++cutoff_array[i];

    // Only evaluate previous runs if we aren't at the root...
    if (phy_node_depth > 0) {
        unsigned int cut_depth = phy_node_depth - 1;

        while ((cut_depth > 0) && (cutoff_array[cut_depth] != 0))
            --cut_depth;

#if DEBUG_PRINTFS
        fprintf(stderr, "Using depth %u = %u - 1 - %u "
                "(= %u ingroup hits) at %p\n", cut_depth, phy_node_depth,
                cut_depth, bgr_node->ingroup_array[cut_depth], bgr_node);
#endif
        if (bgr_node->ingroup_array[cut_depth] == ID_TYPE_UNDEF ) {
            // There has been a cut-off in the phylogenetic root node, if this
            // here happens. This can only mean a cut-off because of too many
            // outgroup hits. So it doesn't make sense to continue the
            // evaluation of child nodes.

            // Update (decrease) the node ingroup array...
            for (unsigned int i = 0; i < phy_node_depth; ++i)
                if (node_ingroup_array[i] == ID_TYPE_UNDEF )
                    --cutoff_array[i];

#if DEBUG_PRINTFS
            fprintf(stderr, "Stopping further evaluation of node %p. "
                    "Root node was cut off.", bgr_node);
#endif
            return;
        }

        // Find the lowest ingroup number we have so far (up to i outgroup hits).
        unsigned int min_match = (unsigned int) (-1);
        for (unsigned int i = 0; i <= max_outgroup_hits; i++)
            min_match = MIN(min_match, cassis_node->num_matches[i]);

        // There should at least be one ingroup hit within the subtree
        // to be of interest, so let us increase our reference to '1'.
        if (min_match == 0)
            ++min_match;

        // Cut the branch, if we cannot expect better results from it.
        if (bgr_node->ingroup_array[cut_depth] < min_match) {
#if DEBUG_PRINTFS
            fprintf(stderr,
                    "Cutting! (1) (min_match=%u, parent_match=%u) at %p\n",
                    min_match,
                    (bgr_node->ingroup_array[phy_node_depth - 1 - cut_depth]),
                    bgr_node);
#endif
            if ((cassis_node->left != NULL) || (cassis_node->right != NULL)) {
                // Propagate the parents value as an upper limit.
                unsigned int upper_limit = bgr_node->ingroup_array[cut_depth];
                struct BgrTreeNode *p = bgr_node->parent;
                while (p != NULL) {
                    int done = 1;
                    if (p->ingroup_array[phy_node_depth] < upper_limit) {
                        p->ingroup_array[phy_node_depth] = upper_limit;
                        done = 0;
                    }
                    if (done)
                        break;
                    p = p->parent;
                }
                bgr_node->ingroup_array[phy_node_depth] = -1;
            }

            // Update (decrease) the node ingroup array...
            for (unsigned int i = 0; i < phy_node_depth; ++i)
                if (node_ingroup_array[i] == ID_TYPE_UNDEF )
                    --cutoff_array[i];

            return;
        }
    }
#endif

    // Prefetch some variables to avoid costly memory accesses in the
    // following while loop.
    unsigned int ingroup_counter = parent_ingroup_counter;
    unsigned int outgroup_counter = parent_outgroup_counter;
    unsigned int node_species_size = bgr_node->species->size();
    unsigned int group_size = cassis_node->group->size();
    const unsigned int *v1 = bgr_node->species->val_ptr();
    const unsigned int *v2 = cassis_node->group->val_ptr();
    unsigned int i1 = 0;
    unsigned int i2 = 0;

    // Count the overlapping entries between our phylogenetic group
    // and the current BgrTreeNode.
    while ((i1 < node_species_size) && (i2 < group_size)) {
        unsigned int v1i1 = v1[i1];
        unsigned int v2i2 = v2[i2];
        if (v1i1 > v2i2) {
            // We have an entry in the PhyNode group which is not in our BgrTreeNode.
            i2++;
        } else if (v1i1 < v2i2) {
            // We have an entry in the BgrTreeNode which is not in our PhyNode group.
            outgroup_counter++;
            i1++;
        } else {
            // We have found an entry in both groups.
            ingroup_counter++;
            i1++;
            i2++;
        }
    }
    outgroup_counter += (node_species_size - i1);

#if DEBUG_PRINTFS
    fprintf(
            stderr,
            "Found %u ingroup, %u/%u outgroup for group %u of size %u (signature hit %u species, first was %u) at %p!\n",
            ingroup_counter, outgroup_counter, max_outgroup_hits,
            cassis_node->node_index, group_size, node_species_size, v1[0],
            bgr_node);
#endif

    // Cut the branch, if we exceed our outgroup hits limit...
    if (outgroup_counter > max_outgroup_hits) {
#ifdef USE_CUTTABLES
#if DEBUG_PRINTFS
        fprintf(stderr, "Cutting! (2) at %p\n", bgr_node);
#endif
        if ((cassis_node->left != NULL) || (cassis_node->right != NULL))
            bgr_node->ingroup_array[phy_node_depth] = ID_TYPE_UNDEF;

        // Update (decrease) the node ingroup array...
        for (unsigned int i = 0; i < phy_node_depth; ++i)
            if (node_ingroup_array[i] == ID_TYPE_UNDEF )
                --cutoff_array[i];
#endif
        return;
    }

    // Add our entry to the result array, if appropriate...
    if ((ingroup_counter > 0) && (bgr_node->supposed_outgroup_matches)
            && (bgr_node->supposed_outgroup_matches->size() > 0)) {
        // Let us evaluate, if at least one of the signatures is worth being
        // added to the results...
        UnorderedIntSet *bgr_node_so = bgr_node->supposed_outgroup_matches;
        for (unsigned int s = 0; s < bgr_node_so->size(); ++s) {
            unsigned int outgroup_sum = bgr_node_so->val(s) + outgroup_counter;

            // ...i.e. below max_outgroup_hits:
            if (outgroup_sum <= max_outgroup_hits) {
                if (ingroup_counter > cassis_node->best_ingroup_coverage)
                    cassis_node->starting_solution = starting_solution;

                cassis_node->addMatching(bgr_node->signatures.str->val(s),
                        ingroup_counter, outgroup_sum);
            }
        }
    }

    /* BEST result update */
#ifdef USE_CUTTABLES
    if ((cassis_node->left != NULL) || (cassis_node->right != NULL)) {
#if DEBUG_PRINTFS
        fprintf(stderr, "Initializing depth %u -> %u (at %p)\n",
                phy_node_depth, ingroup_counter, bgr_node);
#endif

        bgr_node->ingroup_array[phy_node_depth] = ingroup_counter;

        struct BgrTreeNode *p = bgr_node->parent;
        while (p != NULL) {
            int done = 1;

            assert(p->ingroup_array[phy_node_depth] != ID_TYPE_UNDEF);
            if (p->ingroup_array[phy_node_depth] < ingroup_counter) {
                p->ingroup_array[phy_node_depth] = ingroup_counter;
                done = 0;
            }

            if (done)
                break;
            p = p->parent;
        }
    }
#endif
    /* BGRT recursion */
    struct BgrTreeNode *child = bgr_node->children;
    while (child != NULL) {
        traverse_BgrTree_recursion(starting_solution, child, cassis_node,
                cassistree_depth, max_outgroup_hits, cutoff_array,
                ingroup_counter, outgroup_counter, base4_compression);
        child = child->next;
    }

#ifdef USE_CUTTABLES
    // Update (decrease) the node ingroup array...
    for (unsigned int i = 0; i < phy_node_depth; ++i)
        if (node_ingroup_array[i] == ID_TYPE_UNDEF )
            --cutoff_array[i];
#endif
}

#ifdef PTHREADS
/*!
 * Parameters for our pThread
 */
struct BGRTTraversal_work {
    pthread_mutex_t mutex;
    struct BgrTree *bgr_tree;
    CaSSiSTreeNode *curr_cassis_node;
    const CaSSiSTree *tree;
    unsigned int max_outgroup_hits;
    unsigned int remaining;
    unsigned int pos;
};

/*!
 * Traverse a range of BgrTree entries for a given PhyloTree node
 * using pThreads.
 */
void *traverse_BgrTree_pthread(void *ptr) {
    // Fetch pointer to parameter struct.
    struct BGRTTraversal_work *work = (struct BGRTTraversal_work*) ptr;

    // Create a per-thread cutoff array.
    unsigned int *cutoff_array = (unsigned int*) calloc(
            work->tree->tree_depth + 1, sizeof(unsigned int));

    // The threads working loop...
    while (1) {
        // Fetch a BGRT node to process...
        pthread_mutex_lock(&(work->mutex));
        if (work->remaining == 0) {
            // Stop, if no work is remaining...
            pthread_mutex_unlock(&(work->mutex));
            break;
        }
        unsigned int pos = work->pos++;
        work->remaining--;
        pthread_mutex_unlock(&(work->mutex));

        // Process the fetched BGRT node.
        pos = pos % work->bgr_tree->num_species;
        traverse_BgrTree_recursion(pos, work->bgr_tree->nodes[pos],
                work->curr_cassis_node, work->tree->tree_depth + 1,
                work->max_outgroup_hits, cutoff_array, 0, 0,
                work->bgr_tree->base4_compressed);
    }

    free(cutoff_array);
    return NULL;
}

#endif /* #ifdef PTHREADS */

/*!
 * Traverse a BgrTree and evaluate the one node of the PhyloTree.
 *
 * This function is used to fetch the correct starting point from the
 * BgrTree, as the first level of the tree is arranged as an array.
 */
void traverse_BgrTree(struct BgrTree *bgr_tree,
        CaSSiSTreeNode *curr_cassis_node, const CaSSiSTree *cassis_tree,
        unsigned int max_outgroup_hits) {

    // Create a 'cutoff-array'
    unsigned int *cutoff_array = (unsigned int*) calloc(
            cassis_tree->tree_depth + 1, sizeof(unsigned int));

    // If no outgroup hits allowed:
    // Only evaluate nodes from our current phylogenetic group as starting points.
    if (max_outgroup_hits == 0) {
        // Only evaluate nodes from our search group as starting nodes
        for (unsigned int i = 0; i < curr_cassis_node->group->size(); i++) {
            // Fetch a starting solution (position in the BGRT array).
            unsigned int starting_solution = curr_cassis_node->group->val(i);

            // Fetch the BgrTree starting node for our evaluation:
            struct BgrTreeNode *starting_node =
                    bgr_tree->nodes[starting_solution];

            // Use the starting_node for further evaluation...
            traverse_BgrTree_recursion(starting_solution, starting_node,
                    curr_cassis_node, cassis_tree->tree_depth + 1,
                    max_outgroup_hits, cutoff_array, 0, 0,
                    bgr_tree->base4_compressed);
        }
    } else {
        // Starting solution #1
        //
        // Start with the first element in the phylogenetic group.
        unsigned int starting_solution = curr_cassis_node->group->val(0);

        // Starting solution #2 (may override #1)
        //
        // Now check check the results our our phylogenetic parent node
        // (if available) and choose the result with the highest number
        // of ingroup hits as a 'better' starting solution.
        //
        // Fetch the phylogenetic parent node, if available.
        const CaSSiSTreeNode *parent_node = curr_cassis_node->parent;
        if (parent_node) {
            // Fetch (from the parent node) the starting solution
            // with best coverage (number of ingroup hits).
            if (parent_node->starting_solution != ((unsigned int) -1))
                starting_solution = parent_node->starting_solution;
        }
#ifndef PTHREADS
        for (unsigned int i = 0; i < bgr_tree->num_species; ++i) {
            unsigned int pos = (starting_solution + i) % bgr_tree->num_species;
            traverse_BgrTree_recursion(pos, bgr_tree->nodes[pos],
                    curr_cassis_node, cassis_tree->tree_depth + 1,
                    max_outgroup_hits, cutoff_array, 0, 0,
                    bgr_tree->base4_compressed);
        }
#else
        BGRTTraversal_work work;
        pthread_mutex_init(&(work.mutex), (pthread_mutexattr_t*) 0);
        work.bgr_tree = bgr_tree;
        work.curr_cassis_node = curr_cassis_node;
        work.tree = cassis_tree;
        work.max_outgroup_hits = max_outgroup_hits;
        work.remaining = bgr_tree->num_species;
        work.pos = starting_solution;

        for (unsigned int i = 0; i < static_num_processors; i++)
            pool_run(traverse_BgrTree_pthread, &work);

        pool_barrier();
        pthread_mutex_destroy(&(work.mutex));
#endif
    }

#if 0 // DEBUG STUFF...
    unsigned int assert_sum = 0;
    for (int i = 0; i <= cassis_tree->tree_depth; ++i)
        assert_sum += cutoff_array[i];
    assert(assert_sum == 0);
#endif
    free(cutoff_array);
}

/*!
 * Traverse a PhyloTree and evaluate the nodes 'depth first'.
 */
void findTreeSpecificSignatures_recursion(struct BgrTree *bgr_tree,
        const CaSSiSTree *cassis_tree, CaSSiSTreeNode *cassis_node,
        unsigned int max_outgroup_hits) {

#if PROGRESS_METER
    // Display a progress-meter for the tree traversal part.
    static int progress = 0;
    ++progress;
    if ((cassis_tree->num_nodes > 100)
            && (progress % (cassis_tree->num_nodes / 100) == 0))
        fprintf(stdout, "%3d%% done...\n",
                (progress * 100 / cassis_tree->num_nodes));
#endif
    // Evaluate the current node...
    traverse_BgrTree(bgr_tree, cassis_node, cassis_tree, max_outgroup_hits);

    // Make a depth search -> enter left and right branches first.
    if (cassis_node->left) {
        findTreeSpecificSignatures_recursion(bgr_tree, cassis_tree,
                cassis_node->left, max_outgroup_hits);
    }
    if (cassis_node->right) {
        findTreeSpecificSignatures_recursion(bgr_tree, cassis_tree,
                cassis_node->right, max_outgroup_hits);
    }
}

/*!
 * Computes signature candidates for groups and leafs
 * using a BgrTree and a PhyloTree.
 */
bool findTreeSpecificSignatures(struct BgrTree *bgr_tree,
        const CaSSiSTree *cassis_tree, unsigned int max_outgroup_hits) {
    // Return, if one of the trees is undefined.
    if (!bgr_tree || !cassis_tree)
        return NULL;

#ifdef PTHREADS
    // Initialize pThread pool.
    if (static_num_processors == 0)
        static_num_processors = num_processors();

    pool_init(static_num_processors);
#endif

    // Traverse the BgrTree and add the ingroup/outgroup hits
    // for each PhyloTree node.
    findTreeSpecificSignatures_recursion(bgr_tree, cassis_tree,
            cassis_tree->tree_root, max_outgroup_hits);

#ifdef PTHREADS
    // Destroy pThread pool.
    pool_shutdown();
#endif

    return true;
}

/*!
 * Computes group specific signatures based on a given identifier list.
 */
bool findGroupSpecificSignatures(struct BgrTree *bgr_tree, IntSet *ids,
        unsigned int *&num_matches, StrRefSet *&signatures,
        unsigned int max_outgroup_hits) {
    if (!bgr_tree || !ids)
        return false;

    // Create a fake phylogenetic tree node...
    CaSSiSTreeNode *node = new CaSSiSTreeNode(max_outgroup_hits);
    node->group = ids;

    // Create a fake phylogenetic tree node...
    CaSSiSTree *tree = new CaSSiSTree();

    // Evaluation...
    traverse_BgrTree(bgr_tree, node, tree, max_outgroup_hits);

    // Extract the results from the the fake phylogenetic node.
    num_matches = node->num_matches;
    node->num_matches = NULL;
    signatures = node->signatures;
    node->signatures = NULL;

    // Free obsolete phylogenetic tree stuff...
    node->group = NULL;
    delete node;
    delete tree;

    return true;
}

/*!
 * Computes group specific signatures based on a given node.
 * Caution: This is a HACK!
 */
bool findNodeSpecificSignatures(struct BgrTree *bgr_tree, CaSSiSTreeNode *node,
        unsigned int *&num_matches, unsigned int max_outgroup_hits) {
    if (!bgr_tree || !node)
        return false;

    // Create a fake phylogenetic tree node...
    CaSSiSTree *tree = new CaSSiSTree();

    // Evaluation...
    traverse_BgrTree(bgr_tree, node, tree, max_outgroup_hits);

    delete tree;
    return true;
}
