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
#include <cstdio>

static const int fd_in = 0;

/*!
 * Main function.
 * Dumps all incoming (from stdin) DUP index messages as text to stdout.
 */
int main(int, char **) {
    // Various variables (needed for the dump messages)
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
    while (1) {
        fprintf(stdout, "--------------------------------\n");
        DUP_IO_command command = wait_for_command(fd_in);
        switch (command) {
        case DUP_IO_ERROR:
            fprintf(stdout, "DUMP: An unknown error occurred!\n");
            break;
        case DUP_IO_QUIT:
            fprintf(stdout, "DUMP: Received a quit command. Exiting too!\n");
            exit(EXIT_SUCCESS);
        case DUP_IO_ECHO:
            s = recv_echo(fd_in);
            fprintf(stdout, "DUMP: Received an echo string: \"%s\"\n", s);
            free(s);
            break;
        case DUP_IO_SEQ:
            s = recv_seq(fd_in, &id);
            if (s && (id != ID_TYPE_UNDEF)) {
                fprintf(stdout, "DUMP: Received a sequence with ID %d"
                        " and sequence length %ld\n", id, strlen(s));
            } else
                fprintf(stdout, "DUMP: Erroneous sequence package received.\n");
            free(s);
            break;
        case DUP_IO_COMP_IDX:
            fprintf(stdout, "DUMP: Received a 'compute index' command.\n");
            break;
        case DUP_IO_COMP_IDX_DONE:
            fprintf(stdout, "DUMP: Received a 'compute "
                    "index done' notification.\n");
            break;
        case DUP_IO_INIT_SIG:
            if (!recv_init_sig(fd_in, &length, &RNA))
                fprintf(stdout, "DUMP: Received an erroneous 'initialize "
                        "signatures' package (length=%d and RNA=%d)\n", length,
                        RNA);
            else
                fprintf(stdout, "DUMP: Received an 'initialize "
                        "signatures' package with length=%d and RNA=%d\n",
                        length, RNA);
            break;
        case DUP_IO_QRY_NEXT_SIG:
            fprintf(stdout, "DUMP: Received a request for the "
                    "next signature.\n");
            break;
        case DUP_IO_ANS_NEXT_SIG:
            s = recv_ans_next_sig(fd_in);
            fprintf(stdout, "DUMP: Received the"
                    "next signature package: %s\n", s);
            free(s);
            break;
        case DUP_IO_QRY_MATCH_SIG:
            s = recv_qry_match_sig(fd_in, &mm, &mm_dist, &use_wmis);
            fprintf(stdout, "DUMP: Received a match signature request."
                    "\n\tsignature = %s"
                    "\n\tmism. =     %f"
                    "\n\tmm_dist =   %f"
                    "\n\tuse_wmis =  %d", s, mm, mm_dist, use_wmis);
            free(s);
            break;
        case DUP_IO_ANS_MATCH_SIG:
            recv_ans_match_sig(fd_in, set, og_matches);
            fprintf(stdout,
                    "DUMP: Received a match signature answer. %d matches.",
                    set->size());
            break;
        }
    }
    return EXIT_SUCCESS;
}
