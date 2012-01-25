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

#include "parameters.h"
#include <cassis/config.h>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>

Parameters::Parameters() :
m_command(CommandUndef), m_bgrt_file(), m_verbose(false), m_index(
        IndexMiniPt), m_output(OutputClassicCSV), m_seq_files(), m_check_r_c(
                false), m_allowed_mm(0), m_mm_dist(1), m_min_len(18), m_max_len(
                        18), m_use_gc(false), m_min_gc(0.0), m_max_gc(100.0), m_use_tm(
                                false), m_min_tm(-273.0), m_max_tm(273.0), m_use_wm(false), m_num_threads(
                                        0), m_treefile(), m_treename(), m_og_limit(0), m_all_signatures(
                                                false) {
}

Parameters::~Parameters() {

}

void Parameters::reset() {
    m_command = CommandUndef;
    m_verbose = false;
    m_index = IndexMiniPt;
    m_output = OutputClassicCSV;
    m_seq_files.clear();
    m_bgrt_file.clear();
    m_check_r_c = false;
    m_allowed_mm = 0;
    m_mm_dist = 1;
    m_min_len = 18;
    m_max_len = 18;
    m_use_gc = false;
    m_min_gc = 0.0;
    m_max_gc = 100.0;
    m_use_tm = false;
    m_min_tm = -273.0;
    m_max_tm = 273.0;
    m_use_wm = false;
    m_treefile.clear();
    m_og_limit = 0;
    m_num_threads = 0;
    m_all_signatures = false;
}

/*!
 * Dump the parameters to stdout. For debugging purposes.
 */
void Parameters::dump() const {
    std::cout << "Parameters:\n" << "\t-Command       = ";

    switch (m_command) {
    case CommandUndef:
        std::cout << "undefined\n";
        break;
    case CommandHelp:
        std::cout << "help\n";
        break;
    case Command1Pass:
        std::cout << "1pass\n";
        break;
    case CommandCreate:
        std::cout << "create\n";
        break;
    case CommandProcess:
        std::cout << "process\n";
        break;
    case CommandInfo:
        std::cout << "info\n";
        break;
    default:
        std::cout << "???\n";
        break;
    }

    std::cout << "\t-Index         = ";
    switch (m_index) {
    case IndexUndef:
        std::cout << "undefined\n";
        break;
    case IndexMiniPt:
        std::cout << "MiniPt\n";
        break;
    case IndexPtPan:
        std::cout << "PtPan\n";
        break;
    case IndexPtServer:
        std::cout << "PtServer\n";
        break;
    default:
        std::cout << "???\n";
        break;
    }

    std::cout << "\t-Output        = ";
    switch (m_output) {
    case OutputUndef:
        std::cout << "undefined\n";
        break;
    case OutputClassicCSV:
        std::cout << "Classic CSV\n";
        break;
    case OutputDetailedCSV:
        std::cout << "Detailed CSV\n";
        break;
    case OutputTextfiles:
        std::cout << "Textfiles (*.sig)\n";
        break;
    default:
        std::cout << "???\n";
        break;
    }

    std::cout << "\t-Verbose mode  = " << (m_verbose ? "on" : "off") << "\n";

    for (StringList::const_iterator it = m_seq_files.begin();
            it != m_seq_files.end(); it++)
        std::cout << "\t-Database      = \"" << *it << "\"\n";

    std::cout << "\t-BGRT file     = \"" << m_bgrt_file << "\"\n"
            << "\t-Check r.c.    = " << (m_check_r_c ? "yes" : "no") << "\n"
            << "\t-Allowed MM    = " << m_allowed_mm << "\n"
            << "\t-MM dist.      = " << m_mm_dist << "\n"
            << "\t-Oligo. length = " << m_min_len << " -- " << m_max_len << "\n"
            << "\t-Check G+C     = " << (m_use_gc ? "yes" : "no") << "\n"
            << "\t-G+C range     = " << m_min_gc << " -- " << m_max_gc << "\n"
            << "\t-Check temp.   = " << (m_use_tm ? "yes" : "no") << "\n"
            << "\t-Temp. range   = " << m_min_tm << " -- " << m_max_tm << "\n"
            << "\t-Check w.m.    = " << (m_use_wm ? "yes" : "no") << "\n"
            << "\t-All signat.   = " << (m_all_signatures ? "yes" : "no")
            << "\n" << "\t-Treefile      = \"" << m_treefile << "\"\n"
            << "\t-Treename      = \"" << m_treename << "\"\n"
#ifdef PTHREADS
            << "\t-No. threads   = " << m_num_threads << "\n"
#endif
            << "\t-Outg. limit   = " << m_og_limit << "\n\n";
}

bool Parameters::checkIfHelp(const char *c) {
    while (*c == '-' || *c == '/')
        ++c;
    if (!strcmp("help", c))
        return true;
    if (*c == '?' || *c == 'h')
        return true;
    return false;
}

inline bool Parameters::remainingParams(unsigned int argc, unsigned int current,
        unsigned int needed) {
    if (current + needed >= argc)
        return false;
    return true;
}

/*!
 *  Set parameters according to the given strings.
 *  \param argc Number of parameters
 *  \param argv Parameter strings
 *  \return True, if successful, otherwise false if an error occurred.
 */
bool Parameters::set(int argc, char **argv) {
    // If called without enough parameters,
    // CaSSiS assumes that the user needs help.
    if (argc < 3) {
        if ((argc > 1) && checkIfHelp(argv[1])) {
            setCommand(CommandHelp);
            return true;
        }
        std::cerr << "Parameter error: too few arguments. "
                "Try one of these: -? /? -h /h --help\n";
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        // Does the user request help?
        if (checkIfHelp(argv[i])) {
            reset();
            setCommand(CommandHelp);
            return true;
        }

        const char *arg = argv[i];

        if (i == 1) {
            // First argument should be a command...
            if (!strcmp("create", arg))
                setCommand(CommandCreate);
            else if (!strcmp("process", arg))
                setCommand(CommandProcess);
            else if (!strcmp("1pass", arg))
                setCommand(Command1Pass);
            else if (!strcmp("info", arg))
                setCommand(CommandInfo);
            else {
                // This indicates an error...
                std::cerr << "Parameter error: unknown command: " << arg
                        << "\n";
                return false;
            }
        } else {
            // Parameter 2--n: various options and arguments...
            if (*arg == '-' || *arg == '/') {
                // We have identified an option...
                while (*arg == '-' || *arg == '/')
                    ++arg;

                // Parse the Options...
                if (!strcmp("v", arg)) {
                    setVerbose(true);
                } else if (!strcmp("all", arg)) {
                    setAllSignatures(true);
                } else if (!strcmp("idx", arg) && remainingParams(argc, i, 1)) {
                    if (!strcmp("minipt", argv[i + 1]))
                        setIndex(IndexMiniPt);
                    else if (!strcmp("arbpt", argv[i + 1]))
                        setIndex(IndexPtServer);
                    else if (!strcmp("ptpan", argv[i + 1]))
                        setIndex(IndexPtPan);
                    else {
                        setIndex(IndexUndef);
                        std::cerr << "Parameter error: unknown index type.\n";
                        return false;
                    }
                    ++i;

                } else if (!strcmp("out", arg) && remainingParams(argc, i, 1)) {
                    if (!strcmp("classic", argv[i + 1]))
                        setOutput(OutputClassicCSV);
                    else if (!strcmp("detailed", argv[i + 1]))
                        setOutput(OutputDetailedCSV);
                    else if (!strcmp("text", argv[i + 1]))
                        setOutput(OutputTextfiles);
                    else {
                        setOutput(OutputUndef);
                        std::cerr << "Parameter error: unknown output type.\n";
                        return false;
                    }
                    ++i;

                } else if (!strcmp("bgrt", arg)
                        && remainingParams(argc, i, 1)) {
                    if (!setBgrt_file(argv[i + 1])) {
                        std::cerr << "Parameter error: error while parsing "
                                "BGRT filename.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("seq", arg) && remainingParams(argc, i, 1)) {
                    if (!addDB(argv[i + 1])) {
                        std::cerr << "Parameter error: error while parsing "
                                "sequence data filename.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("len", arg) && remainingParams(argc, i, 1)) {
                    const char *dash = strchr(argv[i + 1], '-');
                    if (!dash) {
                        if (!setMin_len(atoi(argv[i + 1]))
                                || !setMax_len(atoi(argv[i + 1]))) {
                            std::cerr << "Parameter error: error while parsing "
                                    "oligonucleotide length.\n";
                            return false;
                        }
                    } else {
                        ++dash;
                        if (!setMin_len(atof(argv[i + 1]))) {
                            std::cerr << "Parameter error: error while parsing "
                                    "min. oligonucleotide length.\n";
                            return false;
                        } else if (!setMax_len(atof(dash))) {
                            std::cerr << "Parameter error: error while parsing "
                                    "max. oligonucleotide length.\n";
                            return false;
                        }
                    }
                    ++i;
                } else if (!strcmp("mis", arg) && remainingParams(argc, i, 1)) {
                    if (!setAllowed_mm(atof(argv[i + 1]))) {
                        std::cerr << "Parameter error: error while parsing "
                                "allowed mismatch parameter.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("dist", arg)
                        && remainingParams(argc, i, 1)) {
                    if (!setMm_dist(atof(argv[i + 1]))) {
                        std::cerr << "Parameter error: error while parsing "
                                "mismatch distance to outgroup.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("wm", arg)) {
                    setUse_wm(true);
                } else if (!strcmp("rc", arg)) {
                    setCheck_r_c(true);
                } else if (!strcmp("gc", arg) && remainingParams(argc, i, 1)) {
                    const char *dash = strchr(argv[i + 1], '-');
                    if (!dash) {
                        std::cerr << "Parameter error: error while parsing "
                                "G+C range.\n";
                        return false;
                    }
                    ++dash;
                    if (!setMin_gc(atof(arg))) {
                        std::cerr << "Parameter error: error while parsing "
                                "min. G+C value.\n";
                        return false;
                    } else if (!setMax_gc(atof(dash))) {
                        std::cerr << "Parameter error: error while parsing "
                                "max. G+C value.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("temp", arg)
                        && remainingParams(argc, i, 1)) {
                    const char *dash = strchr(argv[i + 1], '-');
                    if (!dash) {
                        std::cerr << "Parameter error: error while parsing "
                                "temperature range.\n";
                        return false;
                    }
                    ++dash;
                    if (!setMin_tm(atof(argv[i + 1]))) {
                        std::cerr << "Parameter error: error while parsing "
                                "min. temperature value.\n";
                        return false;
                    } else if (!setMax_tm(atof(dash))) {
                        std::cerr << "Parameter error: error while parsing "
                                "max. temperature value.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("tree", arg)
                        && remainingParams(argc, i, 1)) {
                    char *t_name = strdup(argv[i + 1]);
                    char *colon_pos = NULL;
                    if ((colon_pos = strchr(t_name, ':')) != NULL) {
                        // A tree name was defined.
                        *colon_pos = 0;
                        if (!setTreeFilename(t_name)) {
                            std::cerr << "Parameter error: error while parsing "
                                    "tree file-name.\n";
                            return false;
                        }
                        if (!setTreeName(colon_pos + 1)) {
                            std::cerr << "Parameter error: error while parsing "
                                    "tree name.\n";
                            return false;
                        }
                    } else {
                        if (!setTreeFilename(argv[i + 1])) {
                            std::cerr << "Parameter error: error while parsing "
                                    "tree filename.\n";
                            return false;
                        }
                    }
                    free(t_name);
                    ++i;
                } else if (!strcmp("og", arg) && remainingParams(argc, i, 1)) {
                    if (!setOg_limit(atoi(argv[i + 1]))) {
                        std::cerr << "Parameter error: error while parsing "
                                "outgroup limit.\n";
                        return false;
                    }
                    ++i;
                } else if (!strcmp("par", arg) && remainingParams(argc, i, 1)) {
                    if (!setNum_Threads(atoi(argv[i + 1]))) {
                        std::cerr << "Parameter error: error while parsing "
                                "number of parallel worker threads.\n";
                        return false;
                    }
                    ++i;
                } else {
                    // This indicates an error...
                    std::cerr
                    << "Parameter error: unknown/misplaced/incomplete option: "
                    << arg << "\n";
                    return false;
                }
            } else {
                // This indicates an error...
                std::cerr << "Parameter error: unknown/misplaced parameter: "
                        << arg << "\n";
                return false;
            }
        }
    }
    return true;
}

/*!
 * Dump a usage message.
 *
 * \param verbose Detailed description (on/off)
 */
void Parameters::usage() const {
    std::cout
    << "CaSSiS usage: cassis {1pass|create|process|info} [options]\n"
    "\n"
    "cassis 1pass\n"
    "  Mandatory: -seq [... -seq] -tree\n"
    "  Optional:  -all -dist -gc -idx -len -mis -og -out -rc -temp -wm\n"
    "  Comment:   '1pass' uses the faster CaSSiS-LCA algorithm.\n"
    "\n"
    "cassis create\n"
    "  Mandatory: -bgrt -seq [... -seq]\n"
    "  Optional:  -all -dist -gc -idx -len -mis -rc -temp -wm\n"
    "\n"
    "cassis process\n"
    "  Mandatory: -bgrt -tree\n"
#ifdef PTHREADS
    "  Optional:  -og -out -par\n"
#else
    "  Optional:  -og -out\n"
#endif
    "\n"
    "cassis info\n"
    "  Mandatory: -bgrt\n"
    "\n"
    "Options (alphabetical):\n"
    "  -all              Evaluate all 4^len possible signatures.\n"
    "                    (Not recommended, may take forever... Default: off)\n"
    "  -bgrt <filename>  BGRT file path and name.\n"
    "  -dist <number>    Minimal mismatch distance between a signature candidate\n"
    "                    and non-targets. (Default: 0.0 mismatches)\n"
    "  -gc <min>-<max>   Only allow signatures within a defined G+C content range.\n"
    "                    (Default: 0 -- 100 percent)\n"
    "  -idx <name>       Defines the used search index:\n"
    "                        minipt = \"MiniPt Search Index\" (Default)\n"
#ifdef ARB
    "                        arbpt  = \"ARB PtServer\"\n"
#endif
#ifdef PTPAN
    "                        ptpan  = \"PtPan Search Index\"\n"
#endif
    "  -len {<len>|<min>-<max>}\n"
    "                    Length of the evaluated oligonucleotides. Either a\n"
    "                    fixed length or a range. (Default: 18 bases)\n"
    "  -mis <number>     Number of allowed mismatches within the target group.\n"
    "                    (Default: 1.0 mismatches)\n"
    "  -og <limit>       Number of outgroup hits up to which group signatures are\n"
    "                    computed. (Default: 0)\n"
    "  -out <format>     Defines the output format.\n"
    "                        classic  = \"Classic CSV format\" (Default)\n"
    "                        detailed = \"Detailed CSV format\"\n"
    "                        text     = \"Text file (*.sig) for each group/leaf\"\n"
#ifdef PTHREADS
    "  -par <number>     Number of worker threads (pThreads). Has no influence\n"
    "                    on CaSSiS if pThreads-support is disabled.\n"
#endif
    "  -rc               Drop signatures, if their reverse complement matches\n"
    "                    sequences not matched by the signature itself.\n"
    "                    (Default: off)\n"
#ifdef ARB
    "  -seq <filename>   Sequence data source. This can be an ARB database or a\n"
    "                    MultiFasta file. Multiple sequence sources can be defined.\n"
#else
    "  -seq <filename>   MultiFasta file as sequence data source Multiple sequence\n"
    "                    sources can be defined.\n"
#endif
    "  -temp <min>-<max> Only allow signatures with a melting temperature within\n"
    "                    the defined range. (Default: -273 -- 273 degree Celsius)\n"
#ifdef ARB
    "  -tree <filename|filename.arb:treename>\n"
    "                    Signature candidates will be computed for every defined\n"
    "                    (i.e. named) node within a binary tree. The source can\n"
    "                    either be a Newick tree file or an ARB database\n"
    "                    (an additional tree name is needed here).\n"
#else
    "  -tree <filename>  Signature candidates will be computed for every defined\n"
    "                    (i.e. named) node within a binary tree. Accepts a Newick\n"
    "                    tree file as source.\n"
#endif
    "  -v                Verbose output\n"
    "  -wm               Enable \"weighted mismatch\" values. (Default: off)\n"
    "\n"
    "Caution: Combining the \"-gc\" and \"-temp\" filters can cause unwanted side\n"
    "         effects because they influence each other.\n";
}

/*!
 * Getter methods...
 */
Command Parameters::command() const {
    return this->m_command;
}

bool Parameters::verbose() const {
    return this->m_verbose;
}

Index Parameters::index() const {
    return this->m_index;
}

Output Parameters::output() const {
    return this->m_output;
}

const StringList Parameters::db_files() const {
    return this->m_seq_files;
}

const std::string Parameters::bgrt_file() const {
    return this->m_bgrt_file;
}

bool Parameters::check_r_c() const {
    return this->m_check_r_c;
}

double Parameters::allowed_mm() const {
    return this->m_allowed_mm;
}

double Parameters::mm_dist() const {
    return this->m_mm_dist;
}

unsigned int Parameters::min_len() const {
    return this->m_min_len;
}

unsigned int Parameters::max_len() const {
    return this->m_max_len;
}

bool Parameters::use_gc() const {
    return this->m_use_gc;
}

double Parameters::min_gc() const {
    return this->m_min_gc;
}

double Parameters::max_gc() const {
    return this->m_max_gc;
}

bool Parameters::use_tm() const {
    return this->m_use_tm;
}

double Parameters::min_tm() const {
    return this->m_min_tm;
}

double Parameters::max_tm() const {
    return this->m_max_tm;
}

bool Parameters::use_wm() const {
    return this->m_use_wm;
}

const std::string Parameters::tree_filename() const {
    return this->m_treefile;
}

const std::string Parameters::tree_name() const {
    return this->m_treename;
}

unsigned int Parameters::og_limit() const {
    return this->m_og_limit;
}

unsigned int Parameters::num_threads() const {
    return this->m_num_threads;
}

bool Parameters::allSignatures() const {
    return this->m_all_signatures;
}

/*!
 * Setter methods...
 * Setter return false, if an error occurred, e.g. out of range.
 */
bool Parameters::setCommand(Command c) {
    this->m_command = c;
    return true;
}

bool Parameters::setVerbose(bool v) {
    this->m_verbose = v;
    return true;
}

bool Parameters::setIndex(Index i) {
    this->m_index = i;
    return true;
}

bool Parameters::setOutput(Output o) {
    this->m_output = o;
    return true;
}

bool Parameters::addDB(const std::string &s) {
    this->m_seq_files.push_back(s);
    return true;
}

bool Parameters::setBgrt_file(const std::string &s) {
    if (s.length() == 0) {
        std::cerr << "Parameter error: empty BGRT filename.\n";
        return false;
    }
    this->m_bgrt_file = s;
    return true;
}

bool Parameters::setCheck_r_c(bool c) {
    this->m_check_r_c = c;
    return true;
}

bool Parameters::setAllowed_mm(double a) {
    this->m_allowed_mm = a;
    return true;
}

bool Parameters::setMm_dist(double m) {
    this->m_mm_dist = m;
    return true;
}

bool Parameters::setMin_len(unsigned int m) {
    this->m_min_len = m;
    return true;
}

bool Parameters::setMax_len(unsigned int m) {
    this->m_max_len = m;
    return true;
}

bool Parameters::setUse_gc(bool u) {
    this->m_use_gc = u;
    return true;
}

bool Parameters::setMin_gc(double m) {
    this->m_min_gc = m;
    return true;
}

bool Parameters::setMax_gc(double m) {
    this->m_max_gc = m;
    return true;
}

bool Parameters::setUse_tm(bool u) {
    this->m_use_tm = u;
    return true;
}

bool Parameters::setMin_tm(double m) {
    this->m_min_tm = m;
    return true;
}

bool Parameters::setMax_tm(double m) {
    this->m_max_tm = m;
    return true;
}

bool Parameters::setUse_wm(bool u) {
    this->m_use_wm = u;
    return true;
}

bool Parameters::setTreeFilename(const std::string &s) {
    this->m_treefile = s;
    return true;
}

bool Parameters::setTreeName(const std::string &t) {
    this->m_treename = t;
    return true;
}

bool Parameters::setOg_limit(unsigned int o) {
    this->m_og_limit = o;
    return true;
}

bool Parameters::setNum_Threads(unsigned int t) {
    this->m_num_threads = t;
    return true;
}

bool Parameters::setAllSignatures(bool a) {
    this->m_all_signatures = a;
    return true;
}
