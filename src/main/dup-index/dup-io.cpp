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
#include <fcntl.h>

/*
 * Magic numbers...
 */
const unsigned long magic_header = 0x0CA55150;
const unsigned long magic_quit = 0xDEADDEAD;

/*!
 * Package ID counter
 */
unsigned long ID_counter = 0;

/*
 * DUP-IO package header
 */
struct DUP_IO_header {
    unsigned long magic;
    DUP_IO_command command;
    unsigned long id;
};

int init_fd(int fd) {
    errno = 0;
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1 && errno) {
        if (errno != EBADF) {
            perror("fcntl(F_GETFD)");
            fprintf(stdout, "Error something!\n");
            return -1;
        }
    }
    if (flags & FD_CLOEXEC) {
        return 0;
    } else
        return 1;
}

ssize_t fd_read(int fd, char *buf, size_t size) {
    ssize_t ret = read(fd, buf, size);
    if (ret < 0) {
        if (errno == EINTR)
            return 0;
        fprintf(stderr, "Read failed on fd %d: %s\n", fd, strerror(errno));
        exit(1); // TODO: REALLY EXIT?
    }
    return ret;
}

size_t fd_read_block(int fd, char* buf, size_t size) {
    size_t buf_pos = 0;
    while (buf_pos < size) {
        ssize_t bytes_read = read(fd, &buf[buf_pos], size - buf_pos);
        if (bytes_read < 0) {
            if (errno == EINTR)
                return 0;
            fprintf(stderr, "Read failed on fd %d: %s\n", fd, strerror(errno));
            exit(1);
        } else if (bytes_read == 0) {
            // fprintf(stderr, "EOF\n");
            return 0;
        }
        buf_pos += bytes_read;
    }
    return buf_pos;
}

//int fd_read_block(int fd, char *buffer, size_t size, size_t *position) {
//    ssize_t read_size = fd_read(fd, &buffer[*position], size - (*position));
//    if (read_size == 0)
//        return -2;
//
//    *position += read_size;
//    if (size != *position)
//        return 1;
//    *position = 0;
//    return 0;
//}

size_t fd_write(int fd, const char* buf, size_t size) {
    size_t buf_pos = 0;
    while (buf_pos < size) {
        ssize_t bytes_written = write(fd, &buf[buf_pos], size - buf_pos);
        if (bytes_written < 0) {
            fprintf(stderr, "Write failed on fd %d: %s\n", fd, strerror(errno));
            exit(1); // TODO: REALLY EXIT?
        } else if (bytes_written == 0) {
            return 0;
        }
        buf_pos += bytes_written;
    }
    return size;
}

DUP_IO_command wait_for_command(int fd) {
    DUP_IO_header header;
    size_t bytes_read = fd_read_block(fd, (char*) &header,
            sizeof(DUP_IO_header));
    if ((bytes_read != sizeof(DUP_IO_header)) || (header.magic != magic_header))
        return DUP_IO_ERROR;

    // If it is a 'quit', check if it is valid.
    if (header.command == DUP_IO_QUIT) {
        unsigned long magic = 0;
        bytes_read = fd_read_block(fd, (char*) &magic, sizeof(unsigned long));
        if ((bytes_read != sizeof(unsigned long)) || (magic != magic_quit))
            return DUP_IO_ERROR;
    }

    fprintf(stdout, "Received message with id %ld\n", header.id);

    // Return package type.
    return header.command;
}

bool send_quit(int fd) {
    // Create 'quit' message header.
    DUP_IO_header header;
    header.command = DUP_IO_QUIT;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send magic number.
    if (fd_write(fd, (char*) &magic_quit, sizeof(unsigned long))
            != sizeof(unsigned long))
        return false;

    return true;
}

bool send_echo(int fd, const char *message) {
    if (message == NULL)
        return false;

    // Create 'quit' message header.
    DUP_IO_header header;
    header.command = DUP_IO_ECHO;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Message length.
    size_t len = strlen(message);

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send echo string length.
    if (fd_write(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return false;

    // Send echo string.
    if (fd_write(fd, message, len) != len)
        return false;

    return true;
}

char* recv_echo(int fd) {
    // Read message length.
    size_t len = 0;
    if (fd_read_block(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return NULL;

    // Read message buffer.
    char *buffer = (char*) malloc(len);
    if (buffer == NULL)
        return NULL;
    if (fd_read_block(fd, buffer, len) != len) {
        free(buffer);
        return NULL;
    }

    return buffer;
}
