/*!
 * The Bipartite Graph Representation Tree
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2009-2011
 *     Kai Christian Bader <mail@kaibader.de>
 *     Christian Grothoff <christian@grothoff.org>
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

#include "bgrt.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

/*!
 * Base4 - Constructor.
 */
Base4::Base4() :
m_seq(NULL), m_len(0) {
}

/*!
 * Base4 - Destructor.
 */
Base4::~Base4() {
    free(m_seq);
    m_seq = NULL;
    m_len = 0;
}

/*!
 * Convert string to Base4_t.
 */
void Base4::toBase4(const char *sequence) {
    m_len = (unsigned short) strlen(sequence);

    unsigned int base4_len = m_len / 4;
    if (m_len % 4)
        ++base4_len;

    m_seq = (char*) calloc(base4_len, sizeof(char));
    for (unsigned int i = 0; i < m_len; ++i) {
        char c;
        switch (sequence[i]) {
        case 'a':
        case 'A':
            c = 0x00;
            break;
        case 'c':
        case 'C':
            c = 0x01;
            break;
        case 'g':
        case 'G':
            c = 0x02;
            break;
        case 't':
        case 'T':
        case 'u':
        case 'U':
            c = 0x03;
            break;
        default:
            assert(0);
            // Throw an exception. This should not happen.
            c = 0x00; // Any value here would be wrong...
            break;
        }
        m_seq[i / 4] |= c << ((i % 4) * 2);
    }
}

/*!
 * Convert Base4 to string.
 */
char *Base4::toChar(bool RNA) {
    char *sequence = (char *) malloc(m_len + 1);

    char CONV[4] = { 'A', 'C', 'G', 'T' };
    if (RNA)
        CONV[3] = 'U';

    for (unsigned int i = 0; i < m_len; ++i)
        sequence[i] = CONV[(m_seq[i / 4] >> ((i % 4) * 2)) & 0x03];

    sequence[m_len] = 0x00;
    return sequence;
}

/*!
 * Directly add a base4 encoded string s and its length l.
 * Handle with care!
 */
void Base4::set(char *s, unsigned short l) {
    m_len = l;
    m_seq = s;
}

/*!
 * Return the (uncompressed) sequence length.
 */
unsigned short Base4::len() const {
    return m_len;
}

/*!
 * Return a pointer to the sequence.
 */
const char *Base4::seq() const {
    return m_seq;
}

static void fast_append(IntSet **ss, unsigned int val) {
    IntSet *s = *ss;
    if (NULL == s) {
        s = new IntSet();
        *ss = s;
    }

    s->add(val);
}

/*!
 * Compute the differences between two sets of integers.
 *
 * \param s1 first set to consider
 * \param s2 second set to consider
 * \param s1ms2 set to a set with all elements that are in s1 but not in s2, NULL if that set would be empty
 * \param s2ms1 set to a set with all elements that are in s2 but not in s1, NULL if that set would be empty
 * \param s1is2 set to a set with intersection of s1 and s2, NULL if the intersection is empty
 */
void IntSet_diff(const IntSet *s1, const IntSet *s2, IntSet **s1ms2,
        IntSet **s2ms1, IntSet **s1is2) {
    unsigned int o1;
    unsigned int o2;

    *s1ms2 = NULL;
    *s2ms1 = NULL;
    o1 = 0;
    o2 = 0;
    while ((o1 < s1->size()) || (o2 < s2->size())) {
        if (o1 == s1->size()) {
            fast_append(s2ms1, s2->val(o2));
            o2++;
        } else if (o2 == s2->size()) {
            fast_append(s1ms2, s1->val(o1));
            o1++;
        } else if (s1->val(o1) < s2->val(o2)) {
            fast_append(s1ms2, s1->val(o1));
            o1++;
        } else if (s1->val(o1) > s2->val(o2)) {
            fast_append(s2ms1, s2->val(o2));
            o2++;
        } else {
            fast_append(s1is2, s2->val(o2));
            o1++;
            o2++;
        }
    }
}

/*!
 * Create a new PG tree.
 *
 * \param num_species maximum number of species to support
 * \return NULL on error
 */
struct BgrTree *BgrTree_create(unsigned int num_species,
        bool base4_compressed) {
    struct BgrTree *tree;

    tree = (BgrTree *) calloc(1, sizeof(struct BgrTree));
    tree->nodes = (BgrTreeNode **) calloc(num_species,
            sizeof(struct BgrTreeNode*));
    if (tree->nodes == NULL) {
        BgrTree_destroy(tree);
        return NULL;
    }
    tree->base4_compressed = base4_compressed;
    tree->num_species = num_species;
    tree->min_gc = 0;
    tree->max_gc = 0;
    tree->min_temp = 0;
    tree->max_temp = 0;
    return tree;
}

/*!
 * Free subtree of the PgTree.  Helper-function
 * for BgrTree_destroy.
 *
 * \param node root of the subtree to free
 */
static void free_node(struct BgrTreeNode *node, bool base4_compressed) {
    struct BgrTreeNode *pos;
    struct BgrTreeNode *next;

    if (node == NULL)
        return;
    pos = node->children;
    while (NULL != pos) {
        next = pos->next;
        free_node(pos, base4_compressed);
        pos = next;
    }
    delete node->species;

    if (base4_compressed)
        delete node->signatures.base4;
    else
        delete node->signatures.str;

    delete node->supposed_outgroup_matches;

    free(node->ingroup_array);
    free(node);
}

/*!
 * Free memory occupied by a PG tree.
 *
 * \param tree tree to free
 */
void BgrTree_destroy(struct BgrTree *tree) {
    if (tree == NULL)
        return;

    free(tree->comment);

    for (unsigned int i = 0; i < tree->num_species; i++)
        free_node(tree->nodes[i], tree->base4_compressed);
    free(tree->nodes);
    free(tree);
}

/*!
 * We need to insert the given signature and species
 * list into the subtree of the given parent node.
 *
 * \param parent parent node in the tree; all species
 *        of the ancestors of this node (inclusive)
 *        are matching the signature;
 * \param signature the signature, will be freed with the tree
 * \param species list of species not already given by the
 *        ancestors of parent (inclusive) that also
 *        match 'signature'; guaranteed to be non-empty,
 *        will be freed!
 */
static struct BgrTreeNode *BgrTree_nodelevel_insertion(
        struct BgrTreeNode *parent, IntSet *species, bool base4_compressed) {
    unsigned int first_species;
    struct BgrTreeNode *pos;
    struct BgrTreeNode *prev;
    struct BgrTreeNode *tree_node;
    struct BgrTreeNode *overlap_node;
    IntSet *new_left;
    IntSet *tree_left;
    IntSet *overlap;

    struct BgrTreeNode *return_node = NULL;

    /* find responsible child node with overlap in
     'first_species', or at least position in list
     for insertion */
    first_species = species->val(0);
    prev = NULL;
    pos = parent->children;
    while ((pos != NULL) && (pos->species->val(0) < first_species)) {
        prev = pos;
        pos = pos->next;
    }
    if ((pos != NULL) && (pos->species->val(0) == first_species)) {
        /* 'pos' is where we need to do the insertion
         and an overlap exists; calculate which
         specific type of overlap we are dealing with;
         the logic is virtually identical to that
         in BgrTree_insert for the top-level  */

        new_left = tree_left = overlap = NULL;
        IntSet_diff(species, pos->species, &new_left, &tree_left, &overlap);
        delete species;
        assert(overlap);
        if ((tree_left != NULL) && (new_left != NULL)) {
            /* no subset/superset relationship; create a new overlap
             node and create two child-nodes (with new_left and
             tree_left for the species) */
            overlap_node = (BgrTreeNode *) calloc(1,
                    sizeof(struct BgrTreeNode));
            overlap_node->parent = parent;
            overlap_node->children = pos;
            pos->parent = overlap_node;
            overlap_node->species = overlap;

            if (base4_compressed)
                overlap_node->signatures.base4 = new Base4Set();
            else
                overlap_node->signatures.str = new StrSet();

            overlap_node->supposed_outgroup_matches = new UnorderedIntSet();

            overlap_node->next = pos->next;
            if (prev == NULL)
                parent->children = overlap_node;
            else
                prev->next = overlap_node;

            /* reduce species of first child set since some are now
             covered by the new parent */
            delete pos->species;
            pos->species = tree_left;
            pos->next = NULL;
            /* now insert remaining species as second child;
             re-using code from recursive procedure even
             though we know that here no recursion will happen;
             note that this will result in a redundant re-calculation
             of the (empty) overlap between 'tree_left' and 'new_left',
             so some performance could be gained here through
             specialization in the future.  */
            return_node = BgrTree_nodelevel_insertion(overlap_node, new_left,
                    base4_compressed);
        } else if (tree_left != NULL) {
            /* existing tree is superset (new_left == NULL), create
             a new parent node with the overlapping species and
             make the existing node (pos) a subtree */
            overlap_node = (BgrTreeNode*) calloc(1, sizeof(struct BgrTreeNode));
            overlap_node->children = pos;
            pos->parent = overlap_node;
            overlap_node->parent = parent;
            overlap_node->species = overlap;

            if (base4_compressed)
                overlap_node->signatures.base4 = new Base4Set();
            else
                overlap_node->signatures.str = new StrSet;

            overlap_node->supposed_outgroup_matches = new UnorderedIntSet();

            return_node = overlap_node;

            overlap_node->next = pos->next;
            if (prev == NULL)
                parent->children = overlap_node;
            else
                prev->next = overlap_node;

            /* reduce species of child set since some are now
             covered by the new parent */
            delete pos->species;
            pos->species = tree_left;
            pos->next = NULL;
        } else if (new_left != NULL) {
            /* not all of our species were matched by the
             top-level node; insert the rest somewhere
             within the children of the top-level node
             (need to do more set checks to see where they
             fit; requires recursion) */
            return_node = BgrTree_nodelevel_insertion(pos, new_left,
                    base4_compressed);
            delete overlap;
        } else {
            /* perfect match, only extend signature list */
            delete overlap;
            return_node = pos;
        }
    } else {
        /* species never matched as the lowest-numbered
         species among the children of the given parent;
         create a new child (insert between prev and pos) */
        tree_node = (BgrTreeNode *) calloc(1, sizeof(struct BgrTreeNode));
        tree_node->species = species;
        tree_node->parent = parent;

        if (base4_compressed)
            tree_node->signatures.base4 = new Base4Set();
        else
            tree_node->signatures.str = new StrSet();

        tree_node->supposed_outgroup_matches = new UnorderedIntSet();

        return_node = tree_node;

        tree_node->next = pos;
        if (prev == NULL)
            parent->children = tree_node;
        else
            prev->next = tree_node;
    }

    assert(return_node);
    return return_node;
}

/*!
 * Insert a signature into the tree.
 *
 * \param tree tree to update
 * \param signature signature to store, will be freed with the tree
 * \param species list of species to associate,
 *        will be freed (or kept in the resulting tree)
 * \param supposed_outgroup_matches number of matches m, with m1 < m < m2
 */
static struct BgrTreeNode *BgrTree_treelevel_insertion(struct BgrTree *tree,
        IntSet *species) {
    struct BgrTreeNode *tree_node;
    struct BgrTreeNode *overlap_node;
    IntSet *new_left;
    IntSet *tree_left;
    IntSet *overlap;
    unsigned int first_species;

    struct BgrTreeNode *return_node = NULL;

    first_species = species->val(0);
    tree_node = tree->nodes[first_species];
    if (NULL != tree_node) {
        /* insert here !*/
        new_left = tree_left = overlap = NULL;
        IntSet_diff(species, tree_node->species, &new_left, &tree_left,
                &overlap);
        delete species;
        assert(overlap);
        if ((tree_left != NULL) && (new_left != NULL)) {
            /* no subset/superset relationship; create a new overlap
             node and create two child-nodes (with new_left and
             tree_left for the species) */
            overlap_node = (BgrTreeNode *) calloc(1,
                    sizeof(struct BgrTreeNode));
            overlap_node->parent = tree_node->parent;
            overlap_node->children = tree_node;
            tree_node->parent = overlap_node;
            overlap_node->species = overlap;

            if (tree->base4_compressed)
                overlap_node->signatures.base4 = new Base4Set();
            else
                overlap_node->signatures.str = new StrSet();

            overlap_node->supposed_outgroup_matches = new UnorderedIntSet;

            tree->nodes[first_species] = overlap_node;
            /* reduce species of first child set since some are now
             covered by the new parent */
            delete tree_node->species;
            tree_node->species = tree_left;
            /* now insert remaining species as second child;
             re-using code from recursive procedure even
             though we know that here no recursion will happen;
             note that this will result in a redundant re-calculation
             of the (empty) overlap between 'tree_left' and 'new_left',
             so some performance could be gained here through
             specialization in the future.  */
            return_node = BgrTree_nodelevel_insertion(overlap_node, new_left,
                    tree->base4_compressed);

        } else if (tree_left != NULL) {
            /* existing tree is superset (new_left == NULL), create
             a new parent node with the overlapping species and
             make the existing node a subtree */
            overlap_node = (BgrTreeNode *) calloc(1,
                    sizeof(struct BgrTreeNode));
            overlap_node->parent = tree_node->parent;
            overlap_node->children = tree_node;
            tree_node->parent = overlap_node;
            overlap_node->species = overlap;

            if (tree->base4_compressed)
                overlap_node->signatures.base4 = new Base4Set();
            else
                overlap_node->signatures.str = new StrSet();

            overlap_node->supposed_outgroup_matches = new UnorderedIntSet();

            return_node = overlap_node;

            tree->nodes[first_species] = overlap_node;
            /* reduce species of child set since some are now
             covered by the new parent */
            delete tree_node->species;
            tree_node->species = tree_left;
        } else if (new_left != NULL) {
            /* not all of our species were matched by the
             top-level node; insert the rest somewhere
             within the children of the top-level node
             (need to do more set checks to see where they
             fit; requires recursion) */
            return_node = BgrTree_nodelevel_insertion(tree_node, new_left,
                    tree->base4_compressed);
            delete overlap;
        } else {
            /* perfect match, only extend signature list */
            delete overlap;

            return_node = tree_node;
        }
    } else {
        /* new top-level node needed (species never matched
         as the lowest-numbered species in a species set before) */
        tree_node = (BgrTreeNode *) calloc(1, sizeof(struct BgrTreeNode));
        tree_node->species = species;

        if (tree->base4_compressed)
            tree_node->signatures.base4 = new Base4Set();
        else
            tree_node->signatures.str = new StrSet();

        tree_node->supposed_outgroup_matches = new UnorderedIntSet();

        return_node = tree_node;

        tree->nodes[first_species] = tree_node;
    }

    assert(return_node);
    return return_node;
}

/*!
 * Insert a signature into the tree.
 *
 * \param tree tree to update
 * \param signature signature to store, will be freed with the tree
 * \param species list of species to associate,
 *        will be freed (or kept in the resulting tree)
 * \param supposed_outgroup_matches number of matches m, with m1 < m < m2
 */
void BgrTree_insert(struct BgrTree *tree, const char *signature,
        IntSet *species, unsigned int supposed_outgroup_matches) {
    BgrTreeNode *insert_node = BgrTree_treelevel_insertion(tree, species);

    if (tree->base4_compressed) {
        Base4 *base4 = new Base4();
        base4->toBase4(signature);
        insert_node->signatures.base4->add(base4);
    } else
        insert_node->signatures.str->add(strdup(signature));

    insert_node->supposed_outgroup_matches->add(supposed_outgroup_matches);
}

/*!
 * Insert a signature into the tree.
 *
 * \param tree tree to update
 * \param signatures signature list to store; will be copied.
 * \param species list of species to associate,
 *        will be freed (or kept in the resulting tree)
 * \param supposed_outgroup_matches number of matches m, with m1 < m < m2
 */
void BgrTree_insert(struct BgrTree *tree, StrSet *signatures, IntSet *species,
        UnorderedIntSet *supposed_outgroup_matches) {
    BgrTreeNode *insert_node = BgrTree_treelevel_insertion(tree, species);

    assert(signatures->size() == supposed_outgroup_matches->size());

    for (unsigned int i = 0; i < signatures->size(); ++i) {
        if (tree->base4_compressed) {
            Base4 *base4 = new Base4();
            base4->toBase4(signatures->val(i));
            insert_node->signatures.base4->add(base4);
        } else
            insert_node->signatures.str->add(signatures->val(i));

        insert_node->supposed_outgroup_matches->add(
                supposed_outgroup_matches->val(i));
    }
}
