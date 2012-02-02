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
#include <cstdlib>
#include <iostream>

/*!
 * Default file descriptors
 * '0' = Incoming pipe; from the index client.
 * '1' = Outgoing pipe; to the index client.
 * '2' = Error messages.
 * '3+4', '5+6', ... Incoming and outgoing pipes to filters or index servers.
 */
static int fd_client_recv = 0;
static int fd_client_send = 1;
static int *fd_server_list = NULL;
int fd_server_count = 0;
int fd_server_current = 0;

inline void next_fd_server() {
    fd_server_current += 2;
    if (fd_server_current > fd_server_count)
        fd_server_current = 0;
}

/*
 * The DUP Index filter -- main function.
 */
int main(int argc, char **) {
    // Dump a usage message.
    if (argc > 1) {
        std::cerr << "The DUP Index filter needs no arguments.\n";
        return EXIT_FAILURE;
    }

    // Initialize file descriptors
    init_fd(fd_client_recv);
    init_fd(fd_client_send);
    fd_server_count = init_fd_list(fd_server_list, 3);

    if ((fd_server_count < 2) || (fd_server_count & 1)) {
        std::cerr << "Filter: There is something odd with "
                "the number of server file descriptors.\n";
        return EXIT_FAILURE;
    }

    // Various variables needed by the filter...
    char *s = NULL;
    id_type id = ID_TYPE_UNDEF;
    unsigned int length = 0;
    bool RNA = false;
    //
    const char *s_const = NULL;
    double mm = 0.0;
    double mm_dist = 0.0;
    bool use_wmis = false;
    unsigned int og_matches;
    IntSet *set = new IntSet;

    /*
     * Main loop. Communicates with the client on fd '0' and '1'.
     * Forwards commands/queries to the index server and gathers the results.
     */
    while (1) {
        DUP_IO_command command = wait_for_command(fd_client_recv);
        switch (command) {
        case DUP_IO_ERROR:
            std::cerr << "Filter: An unknown error occurred!\n";
            // return EXIT_FAILURE;
            break;
        case DUP_IO_QUIT:
            std::cout << "Filter: Received a quit command. Exiting!\n";
            free(set);
            return EXIT_SUCCESS;
            break;
        case DUP_IO_ECHO:
            std::cout << "Filter: Receiving an echo.\n";
            s = recv_echo(fd_client_recv);
            std::cout << "Filter: Received echo string: " << s << "\n";
            // Client does _not_ forward an echo. ECHO is considered deprecated!
            free(s);
            break;
        case DUP_IO_SEQ:
            s = recv_seq(fd_client_recv, &id);
            if (s && (id != ID_TYPE_UNDEF)) {
                send_seq(fd_server_list[fd_server_current + 1], id, s);
                next_fd_server();
                free(s);
            } else
                std::cerr << "Filter: Erroneous sequence package received.\n";
            break;
        case DUP_IO_COMP_IDX:
            for (int i = 0; i < fd_server_count; i += 2)
                send_comp_idx(fd_server_list[i + 1]);
            break;
        case DUP_IO_INIT_SIG:
            if (!recv_init_sig(fd_client_recv, &length, &RNA))
                std::cerr << "Filter: Erroneous sequence package received.\n";
            else
                for (int i = 0; i < fd_server_count; i += 2)
                    send_init_sig(fd_server_list[i + 1], length, RNA);
            break;
        case DUP_IO_QRY_NEXT_SIG:
            // s_const = index->fetchNextSignature();
            if (s_const == NULL)
                std::cerr << "Filter: Could not fetch next signature.\n";
            send_ans_next_sig(fd_server_send, s_const);
            break;
        case DUP_IO_ANS_NEXT_SIG:
            std::cerr << "Filter: Error! "
            "A DUP_IO_ANS_NEXT_SIG message was sent to me!\n";
            return EXIT_FAILURE;
            break;
        case DUP_IO_QRY_MATCH_SIG:
            s = recv_qry_match_sig(fd_server_recv, &mm, &mm_dist, &use_wmis);
            if (s == NULL)
                std::cerr << "Filter: Erroneous sequence package received.\n";
            // index->matchSignature(set, s, mm, mm_dist, og_matches, use_wmis);
            free(s);
            if (!send_ans_match_sig(fd_server_send, set, og_matches))
                std::cerr << "Filter: Matched signatures could "
                "not be returned.\n";
            break;
        case DUP_IO_ANS_MATCH_SIG:
            std::cerr << "Filter: Error! "
            "A DUP_IO_ANS_MATCH_SIG message was sent to me!\n";
            return EXIT_FAILURE;
            break;
        }
    }
    free(set);
    return EXIT_SUCCESS;
}
