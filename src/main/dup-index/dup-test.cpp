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

/*
 * DUP test client...
 */
int dup_test_client() {
    return EXIT_SUCCESS;
}

/*
 * DUP test server...
 */
int dup_test_server() {
    return EXIT_SUCCESS;
}

/*
 * Main function.
 */
int main(int argc, char **argv) {
    std::cout << "DUP-IO test...";

    if (argc == 2) {
        if (argv[1][0] == 's' || argv[1][0] == 'S') {
            // Running as server.
            std::cout << " Server!\n";
            return dup_test_server();
        } else if (argv[1][0] == 'c' || argv[1][0] == 'C') {
            // Running as client.
            std::cout << " Client!\n";
            return dup_test_client();
        }
    }

    // An error occurred!
    std::cout << " Missing or wrong argument: "
            "(S)erver or (C)lient! Aborting.\n";
    return EXIT_FAILURE;
}
