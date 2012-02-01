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

#include "dup-index-client.h"
#include "dup-io.h"

/*!
 * File descriptors
 */
const int fd_client_recv = 3;
const int fd_client_send = 4;

/*!
 * Constructor.
 */
DUPIndex::DUPIndex() {
    // Initialize file descriptors
    init_fd(fd_client_recv);
    init_fd(fd_client_send);
}

/*!
 * Destructor.
 */
DUPIndex::~DUPIndex() {

}

/*!
 * A temporary working directory can be defined. This is used to
 * (temporarily) store the files that are computed by the search index.
 * \param directory Path, where (temporary) files can be stored.
 * \return True, if the path is valid, e.g. sufficient space etc...
 */
bool DUPIndex::setTempDir(const char *directory) {
    return false;
}

/*!
 * Adds a sequence to the search index.
 * Adding should fail if an index was already computed.
 * \return True, if the sequence was successfully added.
 */
bool DUPIndex::addSequence(const char *sequence, const id_type id) {
    return send_seq(fd_client_send, id, sequence);
}

/*!
 * This triggers the computation of the search index. It should be called
 * after all sequences were added.
 * \return True, if the index was successfully computed.
 */
bool DUPIndex::computeIndex() {
    return send_comp_idx(fd_client_send);
}

/*!
 * This returns the state the search index is in. As soon as the index is
 * defined as 'computed', no more sequences can be added.
 * \return True, if the index is in the 'computed' state
 * (--> i.e. if the index can process matches.)
 */
bool DUPIndex::isIndexComputed() {
    return false;
}

/*!
 * This is used to initialize the search index when fetching signatures.
 * \param length Length of the signatures that should be returned.
 * \param RNA True, if the index was processed with RNA data.
 * \return False, if an error occurred.
 */
bool DUPIndex::initFetchSignature(unsigned int length, bool RNA) {
    return send_init_sig(fd_client_send, length, RNA);
}

/*!
 * This function returns the next signature that is stored in the search
 * index. The returned signatures should be unique and have at least one
 * match within the index.
 * \return NULL, if an error occurred or nor more signatures are available.
 */
const char *DUPIndex::fetchNextSignature() {
    if (!send_qry_next_sig(fd_client_send))
        return NULL;
    return recv_ans_next_sig(fd_client_recv);
}

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
bool DUPIndex::matchSignature(IntSet *&matched_ids, const char *signature,
        double mm, double mm_dist, unsigned int &og_matches, bool use_wmis) {
    if (!send_qry_match_sig(fd_client_send, signature, mm, mm_dist, use_wmis))
        return false;
    return recv_ans_match_sig(fd_client_recv, matched_ids, og_matches);
}
