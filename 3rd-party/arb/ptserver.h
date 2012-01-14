/*!
 * CaSSiS interface for the ARB PT-Server.
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Parts of the code were taken from: PT_main.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef ARB_PTSERVER_
#define ARB_PTSERVER_

#include <cassis/indexinterface.h>
#include <cassis/types.h>
#include <cassis/namemap.h>

struct ARBPTServer_private;

class ARBPTServer: public IndexInterface {
public:
    ARBPTServer();
    virtual ~ARBPTServer();
    bool setTempDir(const char *directory);
    bool addSequence(const char *sequence, const id_type id);
    bool computeIndex();
    bool isIndexComputed();
    bool initFetchSignature(unsigned int length, bool RNA);
    const char *fetchNextSignature();
    bool matchSignature(IntSet *&matched_ids, const char *signature, double mm,
            double mm_dist, unsigned int &og_matches, bool use_wmis);
    bool fetchMapping(NameMap &map);
private:
    ARBPTServer(const ARBPTServer&);
    ARBPTServer &operator=(const ARBPTServer&);
    const char *find_next_probe_internal();
    ARBPTServer_private *_priv;
    bool is_computed_flag;
    bool has_result_buf;
    char *result_buf;
    char *result_buf_ptr;
};

#endif /* ARB_PTSERVER_ */
