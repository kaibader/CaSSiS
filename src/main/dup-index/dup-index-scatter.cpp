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

///*
// * DUP test client...
// */
//int dup_test_client() {
//    // Initialize file descriptors
//    init_fd(fd_client_recv);
//    init_fd(fd_client_send);
//
//    while (1) {
//        std::cout << "Client: Send commands via keyboard:"
//                "\n\t(e)cho TEXT"
//                "\n\t(s)seq ID SEQUENCE"
//                "\n\t(q)uit"
//                "\n";
//        char c = 0x00;
//        std::cin >> c;
//        if (c == 'q' || c == 'Q') {
//            send_quit(fd_client_send);
//            std::cout << "Client: Sent quit command.\n";
//            break;
//        } else if (c == 'e' || c == 'E') {
//            std::string message;
//            std::cin >> message;
//            send_echo(fd_client_send, message.c_str());
//            std::cout << "Client: Sent echo \"" << message << "\"\n";
//        } else if (c == 's' || c == 'S') {
//            id_type id;
//            std::string seq;
//            std::cin >> id >> seq;
//            send_seq(fd_client_send, id, seq.c_str());
//            std::cout << "Client: Sent sequence " << id << ": " << seq << "\n";
//        }
//    }
//    return EXIT_SUCCESS;
//}
//
///*
// * DUP test server...
// */
//int dup_test_server() {
//    // Initialize file descriptors
//    init_fd(fd_server_recv);
//    init_fd(fd_server_send);
//
//    id_type id;
//    char *message;
//
//    while (1) {
//        DUP_IO_command command = wait_for_command(fd_server_recv);
//        switch (command) {
//        case DUP_IO_ERROR:
//            std::cerr << "Server: An unknown error occurred!\n";
//            // return EXIT_FAILURE;
//            break;
//        case DUP_IO_ECHO:
//            std::cout << "Server: Receiving an echo.\n";
//            message = recv_echo(fd_server_recv);
//            if (message) {
//                std::cout << "Server: Echo message: \"" << message << "\"\n";
//                free(message);
//            } else
//                std::cerr << "Server: No echo message.\n";
//            break;
//        case DUP_IO_SEQ:
//            std::cout << "Server: Receiving a  sequence.\n";
//            message = recv_seq(fd_server_recv, &id);
//            if (message) {
//                std::cout << "Server: sequence " << id << ": " << message << "\n";
//                free(message);
//            } else
//                std::cerr << "Server: No echo message.\n";
//            break;
//        case DUP_IO_QUIT:
//            std::cout << "Server: Received a quit command. Exiting!\n";
//            return EXIT_SUCCESS;
//            break;
//        }
//    }
//    return EXIT_SUCCESS;
//}

/*
 * Main function.
 */
int main(int argc, char **argv) {
    //std::cout << "DUP-IO test...";
    //
    //if (argc == 2) {
    //    if (argv[1][0] == 's' || argv[1][0] == 'S') {
    //        // Running as server.
    //        std::cout << " Server!\n";
    //        return dup_test_server();
    //    } else if (argv[1][0] == 'c' || argv[1][0] == 'C') {
    //        // Running as client.
    //        std::cout << " Client!\n";
    //        return dup_test_client();
    //    }
    //}

    return EXIT_FAILURE;
}
