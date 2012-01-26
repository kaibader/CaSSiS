/*!
 * The CaSSiS Command Line Interface.
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

#include <cassis/config.h>
#include <cassis/search.h>
#include <cassis/io.h>
#include <cassis/namemap.h>
#include <cassis/thermodynamics.h>
#include <cassis/tree.h>

#ifdef ARB
#include <3rd-party/arb/tree.h>
#include <3rd-party/arb/ptserver.h>
#endif

#include <minipt/minipt.h>

#include "csv.h"
#include "complement.h"
#include "fasta.h"
#include "newick.h"
#include "sigfile.h"
#include "parameters.h"
#include "gen-signatures.h"

#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>

#ifdef DUMP_STATS
#include "statistics.h"
#endif

#ifdef PTHREADS
#include <cassis/pool.h>
#endif

/*!
 * FLAG: Dump statistical information about the phylogenetic tree.
 * Enabled by default.
 */
#define DUMP_BGRT_DEPTH 1

/*!
 * This function adds statistical depth information to the BGRTree
 */
unsigned short dumpBGRTDepth_rec(struct BgrTreeNode *node,
        unsigned short parent_depth, unsigned int *nodes) {
    unsigned short depth = parent_depth + 1;

    *nodes = *nodes + 1;

    // Recursion
    struct BgrTreeNode *child = node->children;
    while (child != NULL) {
        unsigned short child_depth = dumpBGRTDepth_rec(child, parent_depth + 1,
                nodes);
        if (depth < child_depth)
            depth = child_depth;
        child = child->next;
    }

    // Return the depth of the subtrees
    return depth;
}

void dumpBGRTDepth(struct BgrTree *bgr_tree) {
    unsigned short tree_depth = 0;
    unsigned int nodes = 0;

    for (unsigned int i = 0; i < bgr_tree->num_species; i++)
        if (bgr_tree->nodes[i]) {
            nodes++;
            unsigned short subtree_depth = dumpBGRTDepth_rec(bgr_tree->nodes[i],
                    0, &nodes);
            if (tree_depth < subtree_depth)
                tree_depth = subtree_depth;
        }
    std::cout << std::endl << "BGRT statistics:"
            << "\n\t- Max. depth (root-depth=0): " << tree_depth
            << "\n\t- Number of nodes: " << nodes << std::endl;
}

/*!
 * Create a search index based on the defined parameters...
 * FIXME: No switch between DNA and RNA. RNA set as default!
 * FIXME: Add a flag that enables name checking (if present in the mapping).
 */
IndexInterface *createIndexInterface(const Parameters &params,
        NameMap &mapping) {
#ifdef DUMP_STATS
    dumpStats("Index: Opening files and adding sequences to index.");
#endif

    // Our search index interface...
    IndexInterface *index = NULL;
    switch (params.index()) {
    case IndexMiniPt:
        index = new MiniPT();
        break;
    case IndexPtServer:
#ifdef ARB
        index = new ARBPTServer();
#else
        std::cerr << "Sorry, but ARB is not supported by "
                "this build of CaSSiS!\n";
        return NULL;
#endif
        break;
    default:
        break;
    }

    if (!index) {
        std::cerr << "Error: Search index not initialized. (unknown reason)\n";
        return NULL;
    }

    // A sequence counter (== number of sequences added to the search index).
    unsigned long counter = 0;

    // Iterate through the database names, open them, and add all found
    // sequences to the search index.
    StringList db_files = params.db_files();
    for (StringList::const_iterator it = db_files.begin(); it != db_files.end();
            it++) {
        std::string lowercase = *it;
        std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
                tolower);
        if (lowercase.find(".arb") != std::string::npos) {
#ifdef ARB
            // Looks like an ARB database file. Open it...
            // if (!addARBSequencesToIndex((*it).c_str(), ptserver)) {
            std::cerr << "An error occurred while trying to add"
                    "the ARB database " << *it << " to the search index.\n";
            delete index;
            return NULL;
            // }
#else
            std::cerr << "Sorry, but this CaSSiS version was built "
                    "without ARB support.\n";
            delete index;
            return NULL;
#endif
        } else {
            // Open a MultiFASTA file...
            FASTA::File *fastafile = new FASTA::File((*it).c_str(), FASTA::RNA);
            if (!fastafile->isOpen()) {
                std::cerr << "An error occurred while trying to add"
                        "the FASTA file " << *it << " to the search index.\n";
                delete index;
                return NULL;
            }
            std::cout << "Fetching sequences from the MultiFASTA file: " << *it
                    << "\n";

            // Fetch the sequences (one by one) and add them to the index.
            while (!fastafile->atEOF()) {
                FASTA::Sequence *sequence = fastafile->getSequence();
                if (sequence) {
                    // Add sequence to the index.
                    char *name_str = sequence->getName();
                    char *seq_str = sequence->getSequence();
                    unsigned int id = mapping.append(name_str);

                    //// FIXME: If 'check mapping is enabled use something like this...
                    //unsigned int id = name_map.id(name_str);
                    //if (id == ID_TYPE_UNDEF) {
                    //    std::cout << "Warning: the sequence name \"" << name_str
                    //            << "is NOT in the phylogenetic tree!\n";
                    //}

                    if (!index->addSequence(seq_str, id)) {
                        std::cerr << "An error occurred while adding the "
                                "sequence: " << name_str << "\n";
                        delete index;
                        return NULL;
                    }

                    free(name_str);
                    free(seq_str);

                    // Free the not anymore needed sequence...
                    delete sequence;

#ifndef NDEBUG
                    // Output a status message if necessary...
                    ++counter;
                    if (counter % 1000 == 0)
                        std::cout << "Sequences added to index... " << counter
                        << "\n";
#endif
                }
            }
            fastafile->close();
            delete fastafile;
        }
    }

#ifdef DUMP_STATS
    dumpStats("Index: All sequences added. Computing the search index.");
#endif

    // Build the search index.
    if (!index->computeIndex()) {
        std::cerr << "An error occurred while computing "
                "the search index.\n";
        delete index;
        return NULL;
    }

#ifdef DUMP_STATS
    dumpStats("Index: Index computed.");
#endif
    return index;
}

/*!
 * Creates a CaSSiSTree structure based on the defined parameters.
 * \param params System parameters
 * \return Pointer to CaSSiSTree if successful, otherwise NULL.
 * FIXME: ARB tree support is missing!
 */
CaSSiSTree *createCaSSiSTree(const Parameters &params) {
#ifdef DUMP_STATS
    dumpStats("Tree: Creating the CaSSiS tree structure.");
#endif

    // Our new tree will be stored here...
    CaSSiSTree *tree = NULL;

    // Create a lower case version of the filename.
    // Needed for ARB database file detection.
    std::string lowercase = params.tree_filename();
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
            tolower);
    if (lowercase.find(".arb") != std::string::npos) {
#ifdef ARB
        // Try to open an ARB database file...
        tree = fetchTreeFromARB(params.tree_filename().c_str(),
                params.tree_name().c_str(), params.og_limit());
#else
        std::cerr << "Sorry, but this CaSSiS version was built "
                "without ARB support.\n";
        return NULL;
#endif
    } else {
        // Try to open a Newick tree file.
        tree = Newick2CaSSiSTree(params.tree_filename().c_str(),
                params.og_limit());
    }

    if (tree == NULL) {
        std::cerr << "File error while creating the CaSSiS tree structure."
                << std::endl;
        return NULL;
    }

    if (params.verbose()) {
        // Print statistics of the phylogenetic tree...
        std::cout << "CaSSiS-Tree statistics:"
                "\n\t- Max. tree depth:              " << tree->tree_depth
                << "\n\t- Number of nodes in tree:      " << tree->num_nodes
                << "\n\t- Number of sequences (leaves): "
                << tree->leaf_mapping.size()
                << "\n\t- Number of defined groups:     "
                << tree->group_mapping.size() << "\n";
    }

#ifdef DUMP_STATS
    dumpStats("Tree: Loaded and processed.");
#endif
    return tree;
}

/*!
 * This function queries a search index and creates the bipartite graph.
 * It either stores the graph as a BGRT or directly in the CaSSiSTree.
 */
int commandCreate1Pass(const Parameters &params) {
#ifdef DUMP_STATS
    dumpStats("Create: Loading CaSSiS tree.");
#endif

    // Our name <--> ID mapping, fetched from the CaSSiSTree.
    NameMap mapping;

    // Create/fetch the CaSSiSTree structure...
    CaSSiSTree *tree = NULL;
    if (params.command() == Command1Pass) {
        tree = createCaSSiSTree(params);
        if (tree == NULL)
            return EXIT_FAILURE;

        tree->fetchMapping(mapping);
    }

#ifdef DUMP_STATS
    dumpStats("Create: CaSSiS tree loaded.");
#endif

    // Our search index interface.
    IndexInterface *index = createIndexInterface(params, mapping);

    if (index == NULL) {
        std::cerr << "Error: Search index not initialized. "
                "Exiting '1pass' strategy.\n";
        return EXIT_FAILURE;
    }

#ifdef DUMP_STATS
    dumpStats("Create: Adding signatures to CaSSiS tree or BGRT.");
#endif

    // Create a new BGRT structure, if requested by the user.
    struct BgrTree *bgr_tree = NULL;
    if (params.command() == CommandCreate)
        bgr_tree = BgrTree_create(mapping.size(), false); // TODO: Support Base4 compression!

    // Class used for thermodynamic calculations...
    Thermodynamics thermo;

    // True, if we need to test a signature against filters...
    bool use_filters = params.use_gc() | params.use_tm();
    if (use_filters) {
        if (params.use_gc())
            thermo.enable_gc_check(params.min_gc(), params.max_gc());
        if (params.use_tm())
            thermo.enable_tm_check(params.min_tm(), params.max_tm());
    }

    // Signatures and matches are stored in here:
    IntSet *matches = NULL;
    const char *signature = NULL;
    GenSignatures genSig;

    // Statistical information about the bipartite graph ic collected here...
    unsigned long stats_edges = 0;
    unsigned long stats_edges_raw = 0;
    unsigned long stats_signatures = 0;
    unsigned long stats_signatures_raw = 0;

    // This is the signature matching process...
    for (unsigned int signature_length = params.min_len();
            signature_length <= params.max_len(); ++signature_length) {

        // Init.: Match signatures of a defined length against the index.
        bool done = false;
        if (params.allSignatures())
            done = genSig.init(signature_length, true);
        else
            done = index->initFetchSignature(signature_length, true);

        if (!done) {
            std::cerr << "An error occurred while "
                    "initializing the signature matching.\n";
            return EXIT_FAILURE;
        }

        if (params.allSignatures())
            signature = genSig.next();
        else
            signature = index->fetchNextSignature();

        // Iterate through all signatures.
        while (signature != NULL) {
            // Test signatures with filters, if necessary...
            // This flag is used to evaluate, if a signature is valid:
            bool signature_valid = true;
            if (use_filters)
                signature_valid = thermo.batch_process(signature);

            if (signature_valid) {
                // Match our signature against the search index and fetch the
                // resulting species IDs. If something went wrong, (e.g.
                // mismatch parameters were not met or no match occurred)
                // 'matched_species' is NULL.
                unsigned int outg_matches = 0;

                index->matchSignature(matches, signature, params.allowed_mm(),
                        params.mm_dist(), outg_matches, params.use_wm());

                if ((matches != NULL) && (matches->size() != 0)) {
                    // Update the statistical information...
                    stats_signatures_raw++;
                    stats_edges_raw += matches->size();

                    // Flags, needed to evaluate the reverse complement (if enabled).
                    bool cmpl_has_matches = false;
                    bool cmpl_matches_subset = false;

                    // Check reverse complement if it was requested via parameter.
                    if (params.check_r_c()) {
                        IntSet *rc_matches = NULL;
                        unsigned int rc_outg_matches = 0;

                        // Fetch the PT-Servers matches for the reverse complement
                        char *rc_signature = reverseComplementSequence(
                                signature, true);
                        index->matchSignature(rc_matches, rc_signature,
                                params.allowed_mm(), params.mm_dist(),
                                rc_outg_matches, params.use_wm());
                        free(rc_signature);

                        if ((rc_matches != NULL) && (rc_matches->size() != 0)) {
                            cmpl_has_matches = true;
                            cmpl_matches_subset = rc_matches->isSubsetOf(
                                    matches);
                        }

                        if (rc_matches != NULL)
                            delete rc_matches;
                    }

                    if ((!cmpl_has_matches) || cmpl_matches_subset) {
                        if (params.command() == Command1Pass) {
                            // Add the signatures directly to the CaSSiSTree,
                            // if we are running a '1Pass' job.
                            if (outg_matches <= params.og_limit())
                                if (tree->addMatching(signature, matches,
                                        outg_matches)) {
                                    // Update the statistical information
                                    // if the matching was added.
                                    stats_signatures++;
                                    stats_edges += matches->size();
                                }
                        } else {
                            // Otherwise build a BGRT by adding the signatures
                            // to it. Also update the statistical information...
                            stats_signatures++;
                            stats_edges += matches->size();

                            BgrTree_insert(bgr_tree, signature, matches,
                                    outg_matches);

                            // The BGRT keeps/manages the matches:
                            // We will clear our pointer reference.
                            matches = NULL;
                        }

                    }
                }
            }
            if (params.allSignatures())
                signature = genSig.next();
            else
                signature = index->fetchNextSignature();
        }
    }

    // Output statistical information about the bipartite graph.
    if (params.verbose())
        std::cout << "Bipartite graph statistics (e=evaluated,a=added):"
        << "\n\t# Edges (e):      " << stats_edges_raw
        << "\n\t# Edges (a):      " << stats_edges
        << "\n\t# Signatures (e): " << stats_signatures_raw
        << "\n\t# Signatures (a): " << stats_signatures << std::endl;

    // Delete matches, if defined.
    delete matches;

    // The search index is not needed anymore.
    delete index;

#ifdef DUMP_STATS
    dumpStats("Create: Done. Signatures added to CaSSiS tree or BGRT.");
#endif

    if (params.command() == Command1Pass) {
        // Fetch the computed results and write the results
        // of our evaluation directly into CSV files.
        if (params.output() == OutputClassicCSV)
            dump2ClassicCSV(tree);
        else if (params.output() == OutputDetailedCSV)
            dump2DetailedCSV(tree);
        else if (params.output() == OutputTextfiles) {
            // Write parameter information into every file, if available.
            std::stringstream comment;
            comment << "\nPresets:\n" << "Length                "
                    << params.min_len() << " -- " << params.max_len() << " nt\n"
                    << "G+C                   " << params.min_gc() << "% -- "
                    << params.max_gc() << "%\n" << "Tm (basic)            "
                    << params.min_tm() << "°C -- " << params.max_tm() << "°C\n"
                    << "Allowed mismatches    " << params.allowed_mm() << "\n"
                    << "Mismatch-distance     " << params.mm_dist();

            dump2Textfiles(tree, comment.str().c_str());
        }
    } else {
        // Otherwise we are creating a BGRT file...

        // Set BGRTree specifications...
        bgr_tree->min_oligo_len = params.min_len();
        bgr_tree->max_oligo_len = params.max_len();
        bgr_tree->min_gc = params.min_gc();
        bgr_tree->max_gc = params.max_gc();
        bgr_tree->min_temp = params.min_tm();
        bgr_tree->max_temp = params.max_tm();
        bgr_tree->ingroup_mismatch_distance = params.allowed_mm();
        bgr_tree->outgroup_mismatch_distance = params.mm_dist();

        // Create a comment...
        std::string comment("Created with CaSSiS from");

        StringList db_files = params.db_files();
        for (StringList::const_iterator it = db_files.begin();
                it != db_files.end(); it++) {
            comment.append(" ");
            comment.append(*it);
        }
#ifdef _WIN32
        comment.append(" on a windows machine.");
#else
        time_t rawtime;
        time(&rawtime);
        comment.append(" on ");
        comment.append(ctime(&rawtime));
#endif
        size_t len = comment.length();
        char *c = (char *) malloc(len);
        memcpy(c, comment.c_str(), len);
        c[len - 1] = 0;
        bgr_tree->comment = c;

        // Write the BGRT file to the disk.
        std::cout << std::endl << "Storing the BGRT in the file \'"
                << params.bgrt_file() << "\'" << std::endl;
        if (!writeBGRTFile(bgr_tree, &mapping, params.bgrt_file().c_str())) {
            std::cerr << "Error while writing the BGRT file onto the disk."
                    << std::endl;
            return EXIT_FAILURE;
        }

        // Statistical information about the BGRT.
        if (params.verbose())
            dumpBGRTDepth(bgr_tree);

        BgrTree_destroy(bgr_tree);
    }

#ifdef DUMP_STATS
    dumpStats("Create: Done. BGRT or results processed and dumped.");
#endif

    if (tree)
        delete tree;
    return EXIT_SUCCESS;
}

/*!
 * CaSSiS 'info' function.
 */
int commandInfo(const Parameters &params, BgrTree *bgr_tree = NULL) {
    NameMap *name_map = NULL;

    if (bgr_tree == NULL) {
        name_map = new NameMap();
        bgr_tree = readBGRTFile(name_map, params.bgrt_file().c_str());
        if (bgr_tree == NULL) {
            // The BGRT file reader was unable to process the file.
            std::cerr << "Error: unable to read the BGRT file.\n";
            return EXIT_FAILURE;
        }
    }

    std::cout << "Loaded BGRT parameters:" << std::endl << "\t- Oligo. length: "
            << bgr_tree->min_oligo_len << "-" << bgr_tree->max_oligo_len
            << " bases" << std::endl << "\t- Ingroup mismatches: "
            << bgr_tree->ingroup_mismatch_distance << std::endl
            << "\t- Mismatch distance to outgroup: "
            << bgr_tree->outgroup_mismatch_distance << std::endl;

    if (bgr_tree->min_gc > 0 || bgr_tree->max_gc < 100)
        std::cout << "\t- G+C range: " << bgr_tree->min_gc << "-"
        << bgr_tree->max_gc << " %" << std::endl;

    if (bgr_tree->min_temp > -270 || bgr_tree->max_temp < 270)
        std::cout << "\t- Melting temp. range: " << bgr_tree->min_temp << "-"
        << bgr_tree->max_temp << " °C" << std::endl;

    if (bgr_tree->comment)
        std::cout << "\t- Comment: " << bgr_tree->comment << std::endl;

#if DUMP_BGRT_DEPTH
    // Statistical data about the BGRTree itself
    dumpBGRTDepth(bgr_tree);
#endif

    // Do some clean-up, if necessary.
    if (name_map != NULL) {
        BgrTree_destroy(bgr_tree);
        delete name_map;
    }

    return EXIT_SUCCESS;
}

/*!
 * This function computes signatures based on a (phylogenetic) tree and a BGRT
 *
 * \param params Program parameters
 * \return Value, that will be returned by the binary after at its exit
 */
int commandProcess(const Parameters &params) {
#ifdef DUMP_STATS
    dumpStats("Process: Loading BGRT and name mapping.");
#endif

    // Our pointer to the name <--> ID map and the BgrTree.
    NameMap *name_map = new NameMap();
    struct BgrTree *bgr_tree = NULL;

    bgr_tree = readBGRTFile(name_map, params.bgrt_file().c_str());
    if (!bgr_tree) {
        // The BGRT file reader was unable to process the file.
        std::cerr << "Error: unable to read the BGRT file.\n";
        return EXIT_FAILURE;
    }

    // If verbose output is enabled: Detailed information about the loaded BGRT.
    if (params.verbose())
        commandInfo(params, bgr_tree);

#ifdef DUMP_STATS
    dumpStats("Process: BGRT loaded.");
#endif

    // Fetch the phylogenetic tree" structure from the ARB database
    std::cout << "Creating the phylogenetic tree structure:" << std::endl;
    CaSSiSTree *tree = createCaSSiSTree(params);
    if (tree == NULL)
        return EXIT_FAILURE;

    // Let the tree use the BGRT mapping.
    if (!tree->enforceExtMapping(*name_map)) {
        std::cerr << "Warning: There is a mismatch in the ID <--> name\n"
                "mapping between the BGRT and the tree!\n";
    }

    // Optimize the phylogenetic tree.
    std::cout << std::endl << "Optimizing the phylogenetic tree "
            "(reducing degenerated branches)..." << std::endl;
    unsigned int old_depth = tree->tree_depth;

    reduceCaSSiSTreeDepth(tree);

    std::cout << "\t- Old depth: " << old_depth << std::endl
            << "\t- New depth: " << tree->tree_depth << std::endl;

#ifdef DUMP_STATS
    dumpStats("Process: Beginning BGRT traversal...");
#endif

    // Traverse the phylogenetic tree...
    findTreeSpecificSignatures(bgr_tree, tree, params.og_limit());

#ifdef DUMP_STATS
    dumpStats("Process: BGRT traversal finished.");
#endif

    // Fetch the computed results and write the results
    // of our evaluation into two different CSV files.
    if (params.output() == OutputClassicCSV)
        dump2ClassicCSV(tree);
    else if (params.output() == OutputDetailedCSV)
        dump2DetailedCSV(tree);
    else if (params.output() == OutputTextfiles) {
        // Write parameter information into every file, if available.
        std::stringstream comment;
        comment << "\nBGRT file:          " << params.bgrt_file()
                        << "\nBGRT comment:       " << bgr_tree->comment;

        dump2Textfiles(tree, comment.str().c_str());
    }

    // Do some clean-up...
    delete tree;
    BgrTree_destroy(bgr_tree);
    delete name_map;

#ifdef DUMP_STATS
    dumpStats("Computation finished.");
#endif

    return EXIT_SUCCESS;
}

/*!
 * Main function.
 */
int main(int argc, char **argv) {
    // The CaSSiS copyright message...
    std::cout << "CaSSiS -- Comprehensive and Sensitive Signature Search\n"
            "Version " << CASSIS_VERSION_MAJOR << "." << CASSIS_VERSION_MINOR
            << "." << CASSIS_VERSION_PATCH<< CASSIS_VERSION_SUFFIX << " ("
            << CASSIS_BUILD_DATE << "), released by Kai Christian Bader.\n\n";

#ifdef DUMP_STATS
    std::cout << "CaSSiS program started.\n";
#endif

    // The program parameters are parsed and stored in here.
    Parameters params;
#ifndef NDEBUG
    std::cout << "This is a debug version of CaSSiS.\n\n";
    bool p_ret = params.set(argc, argv);
    std::cout << "Parameter parser returned with: "
            << (p_ret ? "success" : "error") << "\n";
    if (!p_ret)
        return EXIT_FAILURE;
#else
    std::cout << "\n";
    if (!params.set(argc, argv))
        return EXIT_FAILURE;
#endif

    // Dump verbose information, if defined.
    if(params.verbose())
        params.dump();

#ifdef PTHREADS
    // TODO: This is a 'dirty' hack to set the number of worker threads...
    if (params.num_threads() > 0)
        setNumProcessors(params.num_threads());
    std::cout << "pThread support is enabled.\n";
#endif

#ifdef DUMP_STATS
    std::cout << "Statistical information output is enabled.\n";
#endif

#ifdef ARB
    std::cout << "Compiled with ARB database file "
            "and ARB PT-Server support.\n";
#endif

#ifdef PtPan
    std::cout << "Compiled with PtPan search index support.\n";
#endif

    int returnvalue = EXIT_SUCCESS;

    // Function calls depending on the selected command...
    switch (params.command()) {
    case Command1Pass:
        returnvalue = commandCreate1Pass(params);
        break;
    case CommandCreate:
        returnvalue = commandCreate1Pass(params);
        break;
    case CommandHelp:
        params.usage();
        returnvalue = EXIT_SUCCESS;
        break;
    case CommandInfo:
        returnvalue = commandInfo(params);
        break;
    case CommandProcess:
        returnvalue = commandProcess(params);
        break;
    case CommandUndef:
    default:
        returnvalue = EXIT_FAILURE;
        break;
    }
    return returnvalue;

#ifdef DUMP_STATS
    std::cout << "CaSSiS program ended.\n";
#endif
}
