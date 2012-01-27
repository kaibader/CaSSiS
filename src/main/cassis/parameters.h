/*!
 * Usage and parameter structures and parser
 * (Not generic! Contains predefined parameter settings.)
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

#ifndef CASSIS_PARAMETERS_H_
#define CASSIS_PARAMETERS_H_

#include <string>
#include <list>

typedef std::list<std::string> StringList;

enum Command {
    CommandUndef = 0,
    CommandHelp,
    Command1Pass,
    CommandCreate,
    CommandProcess,
    CommandInfo
};

enum Index {
    IndexUndef = 0, IndexMiniPt, IndexPtServer, IndexPtPan
};

enum Output {
    OutputUndef = 0, OutputClassicCSV, OutputDetailedCSV, OutputTextfiles
};

class Parameters {
public:
    /*!
     * Constructor.
     */
    Parameters();

    /*!
     * Destructor.
     */
    virtual ~Parameters();

    /*!
     *  Reset the parameters.
     */
    void reset();

    /*!
     *  Set parameters according to the given strings.
     *  \param argc Number of parameters
     *  \param argv Parameter strings
     *  \return True, if an error occurred, false if successful.
     */
    bool set(int argc, char **argv);

    /*!
     * Dump a usage message.
     */
    void usage() const;

    /*!
     * Dump the parameters to stdout. For debugging purposes.
     */
    void dump() const;

    /*!
     * Getter methods...
     */
    Command command() const;
    bool verbose() const;
    Index index() const;
    Output output() const;
    const StringList db_files() const;
    const std::string bgrt_file() const;
    bool check_r_c() const;
    double allowed_mm() const;
    double mm_dist() const;
    unsigned int min_len() const;
    unsigned int max_len() const;
    bool use_gc() const;
    double min_gc() const;
    double max_gc() const;
    bool use_tm() const;
    double min_tm() const;
    double max_tm() const;
    bool use_wm() const;
    const std::string tree_filename() const;
    const std::string tree_name() const;
    unsigned int og_limit() const;
    unsigned int num_threads() const;
    bool allSignatures() const;
protected:
    /*!
     * Setter methods...
     * Setter return false, if an error occurred, e.g. out of range.
     */
    bool setCommand(Command c);
    bool setVerbose(bool v);
    bool setIndex(Index i);
    bool setOutput(Output o);
    bool addDB(const std::string &s);
    bool setBgrt_file(const std::string &s);
    bool setCheck_r_c(bool c);
    bool setAllowed_mm(double a);
    bool setMm_dist(double m);
    bool setMin_len(unsigned int m);
    bool setMax_len(unsigned int m);
    bool setUse_gc(bool u);
    bool setMin_gc(double m);
    bool setMax_gc(double m);
    bool setUse_tm(bool u);
    bool setMin_tm(double m);
    bool setMax_tm(double m);
    bool setUse_wm(bool u);
    bool setTreeFilename(const std::string &s);
    bool setTreeName(const std::string &t);
    bool setOg_limit(unsigned int o);
    bool setNum_Threads(unsigned int t);
    bool setAllSignatures(bool a);
private:
    bool checkIfHelp(const char *c);
    inline bool remainingParams(unsigned int argc, unsigned int current,
            unsigned int needed);

    // All parameters
    Command m_command;
    std::string m_bgrt_file;
    bool m_verbose;
    Index m_index;
    Output m_output;
    StringList m_seq_files;
    bool m_check_r_c;
    double m_allowed_mm;
    double m_mm_dist;
    unsigned int m_min_len;
    unsigned int m_max_len;
    bool m_use_gc;
    double m_min_gc;
    double m_max_gc;
    bool m_use_tm;
    double m_min_tm;
    double m_max_tm;
    bool m_use_wm;
    unsigned int m_num_threads;
    std::string m_treefile;
    std::string m_treename;
    unsigned int m_og_limit;
    bool m_all_signatures;
};

#endif /* CASSIS_PARAMETERS_H_ */
