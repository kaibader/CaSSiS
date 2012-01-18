/*!
 * The Bipartite Graph Representation Tree
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2009-2012
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

#ifndef BGRT_H_
#define BGRT_H_

#include "types.h"

#include <cstdlib>

/*!
 * This value is used to mark undefined values/nodes/...
 */
#ifndef ID_TYPE_UNDEF
#define ID_TYPE_UNDEF ((id_type)-1)
#endif

/*!
 * A DNA4/Base4 type (2 bits/char)
 */
class Base4 {
public:
    /*!
     * Base4 - Constructor.
     */
    Base4();

    /*!
     * Base4 - Destructor.
     */
    ~Base4();

    /*!
     * Convert string to DNA4/Base4 encoding.
     */
    void toBase4(const char *sequence);

    /*!
     * Convert DNA4/Base4 to string.
     */
    char *toChar(bool RNA = false);

    /*!
     * Directly add a base4 encoded string s and its length l.
     * Handle with care!
     */
    void set(char *s, unsigned short l);

    /*!
     * Return the (uncompressed) sequence length.
     */
    unsigned short len() const;

    /*!
     * Return a pointer to the sequence.
     */
    const char *seq() const;
protected:
    /*!
     * Sequence string. Base4 encoded. No terminal '0' character.
     */
    char *m_seq;

    /*!
     * Number of base4-encoded characters. Not(!) the length in bytes.
     */
    unsigned short m_len;
private:
    /*!
     * Copy constructor.
     * Not implemented --> private.
     */
    Base4(const Base4&);

    /*!
     * Assignment operator.
     * Not implemented. --> private.
     */
    Base4 &operator=(const Base4&);
};

/*!
 * Set of base4-encoded strings.
 */
typedef USet<Base4*> Base4Set;

/*!
 * Node in the main data structure that we are building
 * (the PG tree).
 */
struct BgrTreeNode {
    /*!
     * Linked list of nodes on the same level (children
     * of the same parent).
     */
    struct BgrTreeNode *next;

    /*!
     * Linked list of children of this node.
     */
    struct BgrTreeNode *children;

    /*!
     * Parent of this node, NULL for the root.
     */
    struct BgrTreeNode *parent;

    /*!
     * Set of species for this node.
     */
    IntSet *species;

    /*!
     * List of signatures for this node.
     */
    union {
        StrSet *str;
        Base4Set *base4;
    } signatures;

    /*!
     * Number of supposed matches m, with m1 < m < m2
     */
    UnorderedIntSet *supposed_outgroup_matches;

    /*!
     * Array used to store the MAXIMUM number of ingroup hits
     * for the BGR-subtree. The index is the depth of the phy_node.
     */
    unsigned int *ingroup_array;
};

/*!
 * Handle to the Bipartite Graph Representation Tree.
 */
struct BgrTree {
    /*!
     * Flag: True, if the signatures are base4 compressed.
     */
    bool base4_compressed;

    /*!
     * Array of "max_species" entries for the top-level nodes.
     */
    struct BgrTreeNode **nodes;

    /*!
     * Maximum number of species (length of the "nodes" array).
     */
    unsigned int num_species;

    /*!
     * Number of ingroup mismatches, that were used to compute the BGRTree.
     */
    unsigned int ingroup_mismatch_distance;

    /*!
     * Mismatch distance that was used to compute the BGRTree.
     */
    unsigned int outgroup_mismatch_distance;

    /*!
     * Minimum and maximum oligonucleotide length in the BGRTree.
     */
    unsigned int min_oligo_len;
    unsigned int max_oligo_len;

    /*!
     * Minimum and maximum G+C content used to filter the BGRTree.
     */
    double min_gc;
    double max_gc;

    /*!
     * Minimum and maximum melting temp. used to filter the BGRTree.
     */
    double min_temp;
    double max_temp;

    /*!
     * A comment that can be added to the BGRTree file.
     */
    char *comment;
};

/*!
 * Create a new PG tree.
 *
 * \param num_species maximum number of species to support
 * \return NULL on error
 */
struct BgrTree *BgrTree_create(unsigned int num_species, bool base4_compressed);

/*!
 * Free memory occupied by a PG tree.
 *
 * \param tree tree to free
 */
void BgrTree_destroy(struct BgrTree *tree);

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
        IntSet *species, unsigned int supposed_outgroup_matches);

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
        UnorderedIntSet *supposed_outgroup_matches);

#endif /* BGRT_H_ */
