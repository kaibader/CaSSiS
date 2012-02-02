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

#include "dup-io.h"

#include <cassis/config.h>
#include <minipt/minipt.h>

#ifdef ARB
#include <arb/ptserver.h>
#endif

#include <iostream>

/*!
 * File descriptors
 */
const int fd_server_recv = 3;
const int fd_server_send = 4;

/*!
 * Main function
 *
 * \param argc Number of program arguments
 * \param argv Program arguments
 * \return Exit code (either EXIT_SUCCESS or EXIT_FAILURE)
 */
int main(int argc, char **) {
    // Dump a usage message.
    if (argc > 1) {
        std::cout << "This is the CaSSiS Index Server.\n";
        return EXIT_FAILURE;
    }

    // Our search_index interface.
    // TODO: Currently the MiniPT index used as default.
    IndexInterface *index = new MiniPT;

    // Initialize file descriptors
    init_fd(fd_server_recv);
    init_fd(fd_server_send);

    // Various variables needed for queries...
    const char *s_const = NULL;
    char *s = NULL;
    id_type id = ID_TYPE_UNDEF;
    unsigned int length = 0;
    bool RNA = false;
    double mm = 0.0;
    double mm_dist = 0.0;
    bool use_wmis = false;
    unsigned int og_matches;
    IntSet *set = new IntSet;

    // Main server loop.
    // Awaits incoming commands on pipe '3' and output goes to pipe '4'.
    while (1) {
        DUP_IO_command command = wait_for_command(fd_server_recv);
        switch (command) {
        case DUP_IO_ERROR:
            std::cerr << "Server: An unknown error occurred!\n";
            // return EXIT_FAILURE;
            break;
        case DUP_IO_QUIT:
            std::cout << "Server: Received a quit command. Exiting!\n";
            free(set);
            delete index;
            return EXIT_SUCCESS;
            break;
        case DUP_IO_ECHO:
            std::cout << "Server: Receiving an echo.\n";
            s = recv_echo(fd_server_recv);
            std::cout << "Server: Received echo string: " << s << "\n";
            free(s);
            break;
        case DUP_IO_SEQ:
            s = recv_seq(fd_server_recv, &id);
            if (s && (id != ID_TYPE_UNDEF))
                index->addSequence(s, id);
            else
                std::cerr << "Server: Erroneous sequence package received.\n";
            break;
        case DUP_IO_COMP_IDX:
            if (!index->computeIndex())
                std::cerr << "Server: Building index failed.\n";
            send_comp_idx_done(fd_server_send);
            break;
        case DUP_IO_COMP_IDX_DONE:
            std::cerr << "Server: Error! "
            "A DUP_IO_COMP_IDX_DONE message was sent to me!\n";
            return EXIT_FAILURE;
            break;
        case DUP_IO_INIT_SIG:
            if (!recv_init_sig(fd_server_recv, &length, &RNA))
                std::cerr << "Server: Erroneous sequence package received.\n";
            else
                index->initFetchSignature(length, RNA);
            break;
        case DUP_IO_QRY_NEXT_SIG:
            s_const = index->fetchNextSignature();
            if (s_const == NULL)
                std::cerr << "Server: Could not fetch next signature.\n";
            send_ans_next_sig(fd_server_send, s_const);
            break;
        case DUP_IO_ANS_NEXT_SIG:
            std::cerr << "Server: Error! "
            "A DUP_IO_ANS_NEXT_SIG message was sent to me!\n";
            return EXIT_FAILURE;
            break;
        case DUP_IO_QRY_MATCH_SIG:
            s = recv_qry_match_sig(fd_server_recv, &mm, &mm_dist, &use_wmis);
            if (s == NULL)
                std::cerr << "Server: Erroneous sequence package received.\n";
            index->matchSignature(set, s, mm, mm_dist, og_matches, use_wmis);
            free(s);
            if (!send_ans_match_sig(fd_server_send, set, og_matches))
                std::cerr << "Server: Matched signatures could "
                "not be returned.\n";
            break;
        case DUP_IO_ANS_MATCH_SIG:
            std::cerr << "Server: Error! "
            "A DUP_IO_ANS_MATCH_SIG message was sent to me!\n";
            return EXIT_FAILURE;
            break;
        }
    }
    free(set);
    return EXIT_SUCCESS;
}

