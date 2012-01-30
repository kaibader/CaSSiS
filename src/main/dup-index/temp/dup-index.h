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

#ifndef DUP_INDEX_H_
#define DUP_INDEX_H_

#include <cstdio>
#include <cerrno>

///*
// * Hardcoded file descriptors.
// * The dup-index clients sends commands/requests to all via 'fd_client_fanout'
// * and sends via round robin on 'fd_client_deal'.
// * It receives responses on 'fd_client_in'.
// */
//const int fd_client_fanout = 3;
//const int fd_client_deal = 4;
//const int fd_client_in = 5;

/*
 * Hardcoded file descriptors.
 * The dup-index server awaits commands/requests on 'fd_server_in' and sends
 * responses on 'fd_server_out'.
 */
const int fd_server_in = 4;
const int fd_server_out = 5;

/*
 * A 'magic' number that is added to 'quit' commands.
 */
const size_t magic_quit_number = 0x12342468;

/*
 * DUP Index commands
 */
enum DUPIndex_command {
    DUPINDEX_QUIT = 0, DUPINDEX_ECHO
};

/*
 * DUP Index package header
 */
struct DUPIndex_header {
    DUPIndex_command command;
    size_t id;
    size_t size;
};

struct DUPIndex_quit {
    DUPIndex_header header;
    size_t magic;
};

struct DUPIndex_echo {
    DUPIndex_header header;
    size_t message_id;
};

ssize_t DUPIndex_read(int fd, char *buf, size_t size) {
    ssize_t ret;

    ret = read(fd, buf, size);
    if (ret < 0) {
        if (errno == EINTR)
            return 0; /* retry */
        fprintf(stderr, "read failed on fd %d: %s\n", fd, strerror(errno));
        exit(1);
    }
    return ret;
}

int DUPIndex_read_block(int fd, char *buffer, size_t buf_size,
        size_t *position) {
    ssize_t read_size = DUPIndex_read(fd, &buffer[*position],
            buf_size - (*position));
    if (read_size == 0)
        return -2;

    *position += read_size;
    if (buf_size != *position)
        return 1;
    *position = 0;
    return 0;
}

#endif /* DUP_INDEX_H_ */
