/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Parts of the code were taken from: PT_main.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef MINIPT_H_
#define MINIPT_H_

#include <cassis/indexinterface.h>

// Forward declaration. Avoids 'contamination' with (global) legacy variables.
class minipt_private;

class MiniPT: public IndexInterface {
public:
    /*!
     * Constructor.
     */
    MiniPT();

    /*!
     * Destructor.
     */
    virtual ~MiniPT();

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
    MiniPT(const MiniPT&);
    MiniPT &operator=(const MiniPT&);
    //
    minipt_private *_priv;
    bool is_computed_flag;
    bool has_result_buf;
    char *result_buf;
    char *result_buf_ptr;
};

#endif /* MINIPT_H_ */
