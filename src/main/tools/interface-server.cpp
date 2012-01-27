/*!
 * Search Index Server -- Accepts commands from stdin/stdout
 * Provides access to search indices via the IndexInterface class.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2011,2012
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
#include <cassis/namemap.h>
#include <minipt/minipt.h>

//#ifdef ARB
//#include <arb/ptserver.h>
//#endif
//
//#include <fasta.h>
//#include <cstring>
// #include <string>

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <string>

/*!
 * Main function
 *
 * \param argc Number of program arguments
 * \param argv Program arguments
 * \return Exit code (either EXIT_SUCCESS or EXIT_FAILURE)
 */
int main(int argc, char **/*argv*/) {
    // This name mapping is used for this server.
    NameMap name_map;

    // Dump a usage message.
    if (argc > 1) {
        std::cout << "This is the CaSSiS Index Server.\n";
        return EXIT_FAILURE;
    }

    // Our search_index interface.
    // TODO: Currently the MiniPT index is hardcoded.
    IndexInterface *search_index = new MiniPT;

    // Open socket connection.
    // Awaits incoming commands on pipe '3' and output goes to pipe '4'.
    FILE *fd_in = fopen(..., "r");
    FILE *fd_in = fopen(..., "r");

    // Main server loop.
    while (1) {

        //...
        break;
    }

    //std::string buffer;
    //std::getline(std::cin, buffer);
    //while (buffer != "quit") {
    //// Fetch single words from the input.
    //std::istringstream iss(buffer);
    //std::string substr;
    //iss >> substr;
    //while (iss) {
    //if(substr == "id") {
    //// We have a new id <-->
    //}
    //
    //// Fetch next word.
    //iss >> substr;
    //}
    //
    //// Fetch next line.
    //std::getline(std::cin, buffer);
    //}

    //if (argc > 1) {
    //    usage();
    //    return 1;
    //}
    //string buf_1;
    //getline(cin, buf_1);
    //while (buf_1 != "quit") {
    //    Parameters params;
    //    istringstream iss(buf_1);
    //    string buf_2;
    //    iss >> buf_2;
    //    while (iss) {
    //        // Parse parameters
    //        if (buf_2 == "id")
    //            iss >> params.SERVERID;
    //        if (buf_2 == "dmaxhits")
    //            iss >> params.DESIGNCPLIPOUTPUT;
    //        if (buf_2 == "dnames")
    //            iss >> params.DESIGNNAMES;
    //        if (buf_2 == "dseq")
    //            iss >> params.DESIGNSEQUENCE;
    //        if (buf_2 == "dprobelen")
    //            iss >> params.DESIGNPROBELENGTH;
    //        if (buf_2 == "dmintmp")
    //            iss >> params.MINTEMP;
    //        if (buf_2 == "dmaxtmp")
    //            iss >> params.MAXTEMP;
    //        if (buf_2 == "dmingc")
    //            iss >> params.MINGC;
    //        if (buf_2 == "dmaxgc")
    //            iss >> params.MAXGC;
    //        if (buf_2 == "dmaxbnd")
    //            iss >> params.MAXBOND;
    //        if (buf_2 == "dminpos")
    //            iss >> params.MINPOS;
    //        if (buf_2 == "dmaxpos")
    //            iss >> params.MAXPOS;
    //        if (buf_2 == "dmishit")
    //            iss >> params.MISHIT;
    //        if (buf_2 == "dtrgts")
    //            iss >> params.MINTARGETS;
    //        if (buf_2 == "mseq")
    //            iss >> params.SEQUENCE;
    //        if (buf_2 == "mmis")
    //            iss >> params.MISMATCHES;
    //        if (buf_2 == "mcmpl")
    //            iss >> params.COMPLEMENT;
    //        if (buf_2 == "mwght")
    //            iss >> params.WEIGHTED;
    //        iss >> buf_2;
    //    }
    //
    //    /*if ((params.DESIGNNAMES.length() > 0) || params.sequence) {
    //     AP_probe_design_event(params);
    //     } else {*/
    //    AP_probe_match_event(params);
    //    /*}*/
    //
    //    getline(cin, buf_1);
    //}
    //return 0;

    // Quit/exit the server.
    return EXIT_SUCCESS;
}

///**************************************************************************/
///**************************************************************************/
///**************************************************************************/
//
///*!
// * Program parameter struct.
// */
//typedef struct {
//
//} Parameters;
//
///*!
// * Main function
// *
// * \param argc Number of program arguments
// * \param argv Program arguments
// * \return Exit code (0 on success)
// */
//int old_main(int argc, char **argv) {
//    // std::cout << "Search Index Server -- Provides access to search indices.\n";
//
//    // Check for correct number of parameters.
//    if (argc < 3 || argc > 4) {
//        std::cout << "Usage:\t" << argv[0]
//                                        << " <index-type> <sequences.fasta> [signature]\n"
//                                        "Types:\tminipt = \"MiniPt Search Index\""
//#ifdef ARB
//                                        "\n\tarbpt  = \"ARB PtServer\""
//#endif
//#ifdef PTPAN
//                                        "\n\tptpan  = \"PtPan Search Index\""
//#endif
//                                        "\n";
//        return EXIT_FAILURE;
//    }
//
//    FASTA::File *fastafile = NULL;
//
//    // This name mapping is used throughout the MiniPT Server...
//    NameMap name_map;
//
//    // Our search_index interface.
//    IndexInterface *search_index = NULL;
//
//    // Use the appropriate search index...
//    if (!strcmp(argv[1], "minipt")) {
//        // Our search index: the MiniPT Index.
//        std::cout << "Using the MiniPt Search Index...\n";
//        search_index = new MiniPT();
//    } else if (!strcmp(argv[1], "arbpt")) {
//#ifdef ARB
//        // Use the ARB PT-Server as search index.
//        std::cout << "Using the ARB PTServer Index...\n";
//        search_index = new ARBPTServer();
//#else
//        std::cerr << "Error: \" The ARB PTServer Index is not available.\n";
//        return EXIT_FAILURE;
//#endif
//    } else if (!strcmp(argv[1], "ptpan")) {
//#ifdef PTPAN
//        std::cout << "Using the PtPan Search Index...\n";
//#else
//        std::cerr << "Error: \" The PtPan Search Index is not available.\n";
//        return EXIT_FAILURE;
//#endif
//    } else {
//        std::cerr << "Error: \"" << argv[1]
//                                         << "\" is not a defined search index.\n";
//        return EXIT_FAILURE;
//    }
//
//    if (!search_index) {
//        std::cerr << "Error: \" Search index was not initialized correctly.\n";
//        return EXIT_FAILURE;
//    }
//
//#if 0
//    // Use the ARB PT-Server as search index.
//    search_index = new ARBPTServer();
//
//    // Compute the ARB PT Server from the given ARB file.
//    if (!search_index->loadIndexFromFile(argv[1])) {
//        fprintf(stderr, "An error occurred while opening "
//                "the ARB database file: %s\n", argv[1]);
//        delete search_index;
//        return EXIT_FAILURE;
//    }
//
//    // Build the search index.
//    if (!search_index->computeIndex()) {
//        fprintf(stderr, "An error occurred while computing "
//                "the search index.\n");
//        delete search_index;
//        return EXIT_FAILURE;
//    }
//
//    // Fetch the ARB PT-Servers name mapping.
//    search_index->fetchMapping(name_map);
//#endif
//
//    // Load the sequence data from the given Fasta file source.
//    fastafile = new FASTA::File(argv[2], FASTA::RNA);
//    if (!fastafile->isOpen()) {
//        std::cerr << "An error occurred while trying "
//                "to open the Fasta file: " << argv[2] << "\n";
//        delete search_index;
//        return EXIT_FAILURE;
//    }
//
//    // Just a sequence counter...
//    unsigned int counter = 0;
//
//    // Add the sequences to the index.
//    while (!fastafile->atEOF()) {
//        FASTA::Sequence *sequence = fastafile->getSequence();
//        if (!sequence) {
//            std::cerr << "Unable to fetch sequence "
//                    "data from the Fasta file\n";
//            delete fastafile;
//            delete search_index;
//            return EXIT_FAILURE;
//        }
//
//        // Output a status message if necessary...
//        ++counter;
//        if (counter % 1000 == 0)
//            std::cout << "building index... " << counter << "\n";
//        // Add sequence to the index.
//        char *name_str = sequence->getName();
//        char *seq_str = sequence->getSequence();
//        if (name_str == NULL || seq_str == NULL) {
//            std::cerr << "Invalid name identifier or sequence.\n";
//        } else {
//            id_type id = name_map.append(name_str);
//            if (!search_index->addSequence(seq_str, id)) {
//                std::cerr << "An error occurred while adding the "
//                        "sequence: " << name_str << "\n";
//                delete fastafile;
//                delete search_index;
//                return EXIT_FAILURE;
//            }
//        }
//        free(name_str);
//        free(seq_str);
//
//        // Free the not anymore needed sequence...
//        delete sequence;
//    }
//
//    // Build the search index.
//    if (!search_index->computeIndex()) {
//        std::cerr << "An error occurred while computing "
//                "the search index.\n";
//        delete fastafile;
//        delete search_index;
//        return EXIT_FAILURE;
//    }
//
//    if (argc == 3) {
//        if (!search_index->initFetchSignature(15, true)) {
//            std::cerr << "An error occurred while "
//                    "initializing the signature matching.\n";
//            delete fastafile;
//            delete search_index;
//            return EXIT_FAILURE;
//        }
//        IntSet *matchlist = NULL;
//        const char *signature = search_index->fetchNextSignature();
//        while (signature != NULL) {
//            std::cout << "\n";
//            unsigned int outgroup_matches = 0;
//
//            search_index->matchSignature(matchlist, signature, 1.0, 1.0,
//                    outgroup_matches, true);
//
//            std::cout << "Signature \"" << signature << "\" was found in "
//                    << matchlist->size() << " sequences.\n";
//            for (unsigned int i = 0; i < matchlist->size(); ++i)
//                std::cout << "-> \"" << name_map.name(matchlist->val(i)).c_str()
//                << "\"\n";
//
//            signature = search_index->fetchNextSignature();
//        }
//        delete matchlist;
//    } else {
//        unsigned int outgroup_matches = 0;
//        IntSet *matchlist = NULL;
//        search_index->matchSignature(matchlist, argv[3], 1.0, 1.0,
//                outgroup_matches, true);
//
//        std::cout << "Signature \"" << argv[3] << "\" was found in "
//                << matchlist->size() << " sequences.\n";
//
//        for (unsigned int i = 0; i < matchlist->size(); ++i)
//            std::cout << "-> \"" << name_map.name(matchlist->val(i)).c_str()
//            << "\"\n";
//
//        delete matchlist;
//    }
//
//    delete search_index;
//
//    // Close & free everything...
//    if (fastafile) {
//        fastafile->close();
//        delete fastafile;
//    }
//    return EXIT_SUCCESS;
//}
