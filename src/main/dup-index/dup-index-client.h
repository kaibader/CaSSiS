/*!
 * DUP Search Index
 * Provides remote access to search indices via the IndexInterface class.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS).
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

#ifndef DUP_INDEX_H_
#define DUP_INDEX_H_

#include <cassis/indexinterface.h>

static const unsigned int fd_server_send = 4;

/*
 * DUP Remote Search Index (Client Interface)
 */
class DUPIndexInterface: public IndexInterface {
public:
    /*!
     * Constructor.
     */
    DUPIndexInterface();

    /*!
     * Destructor.
     */
    virtual ~DUPIndexInterface();

    /*!
     * A temporary working directory can be defined. This is used to
     * (temporarily) store the files that are computed by the search index.
     * \param directory Path, where (temporary) files can be stored.
     * \return True, if the path is valid, e.g. sufficient space etc...
     */
    bool setTempDir(const char *directory);

    /*!
     * Adds a sequence to the search index.
     * Adding should fail if an index was already computed.
     * \return True, if the sequence was successfully added.
     */
    bool addSequence(const char *sequence, const id_type id);

    /*!
     * This triggers the computation of the search index. It should be called
     * after all sequences were added.
     * \return True, if the index was successfully computed.
     */
    bool computeIndex();

    /*!
     * This returns the state the search index is in. As soon as the index is
     * defined as 'computed', no more sequences can be added.
     * \return True, if the index is in the 'computed' state
     * (--> i.e. if the index can process matches.)
     */
    bool isIndexComputed();

    /*!
     * This is used to initialize the search index when fetching signatures.
     * \param length Length of the signatures that should be returned.
     * \param RNA True, if the index was processed with RNA data.
     *
     * TODO: The RNA parameter should be defined when creating the index, not here!
     *
     * TODO: Add a parameter that defines the number of allowed mismatches.
     * Otherwise only signatures without mismatches will be returned?
     *
     * \return False, if an error occurred.
     */
    bool initFetchSignature(unsigned int length, bool RNA);

    /*!
     * This function returns the next signature that is stored in the search
     * index. The returned signatures should be unique and have at least one
     * match within the index.
     * \return NULL, if an error occurred or nor more signatures are available.
     */
    const char *fetchNextSignature();

    /*!
     * Match a signature string against the search index.
     * \param matched_ids Reference to an IntSet where the IDs of matching
     * sequences will be stored. Old content will be cleared.
     * \param signature Signature string that should be matched.
     * \param mm Number of allowed mismatches. (Should be 0 by default.)
     * \param mm_dist Minimum distance to non-target matches.
     * (Should be 1 by default.)
     * \param og_matches Number of allowed outgroup matches. (0 by default.)
     * \param use_wmis Use weighted mismatch values (for mm and mm_dist).
     * \return True, if the match was successfully processed.
     */
    bool matchSignature(IntSet *&matched_ids, const char *signature, double mm,
            double mm_dist, unsigned int &og_matches, bool use_wmis);
private:
    /*!
     * Private copy constructor and assignment operator.
     */
    DUPIndexInterface(const DUPIndexInterface&);
    DUPIndexInterface &operator=(const DUPIndexInterface&);
};

#endif /* DUP_INDEX_H_ */
