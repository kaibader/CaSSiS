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
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>

ssize_t fd_read(int fd, char *buf, size_t buf_size) {
    ssize_t ret = read(fd, buf, buf_size);
    if (ret < 0) {
        if (errno == EINTR)
            return 0;
        fprintf(stderr, "Read failed on fd %d: %s\n", fd, strerror(errno));
        exit(1);
    }
    return ret;
}

int fd_read_block(int fd, char *buffer, size_t buf_size, size_t *position) {
    ssize_t read_size = fd_read(fd, &buffer[*position], buf_size - (*position));
    if (read_size == 0)
        return -2;

    *position += read_size;
    if (buf_size != *position)
        return 1;
    *position = 0;
    return 0;
}

/*
 * DUP-IO package header
 */
struct DUP_IO_header {
    DUP_IO_command command;
    size_t id;
    size_t size;
};

struct DUP_IO_quit {
    DUP_IO_header header;
    size_t magic;
};

struct DUP_IO_echo {
    DUP_IO_header header;
    size_t message_id;
};
