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
    char *buffer = (char*) malloc(len + 1);
    if (buffer == NULL)
        return NULL;
    if (fd_read_block(fd, buffer, len) != len) {
        free(buffer);
        return NULL;
    }

    buffer[len] = 0;
    return buffer;
}

bool send_seq(int fd, id_type id, const char *seq) {
    if ((id == ID_TYPE_UNDEF) || (seq == NULL))
        return false;

    // Create 'add sequence' message header.
    DUP_IO_header header;
    header.command = DUP_IO_SEQ;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send ID.
    if (fd_write(fd, (char*) &id, sizeof(id_type)) != sizeof(id_type))
        return false;

    // Send sequence length.
    size_t len = strlen(seq);
    if (fd_write(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return false;

    // Send sequence.
    if (fd_write(fd, seq, len) != len)
        return false;

    return true;
}

char *recv_seq(int fd, id_type *id) {
    // Read ID.
    if (fd_read_block(fd, (char*) id, sizeof(id_type)) != sizeof(id_type)) {
        *id = ID_TYPE_UNDEF;
        return NULL;
    }

    // Read sequence length.
    size_t len = 0;
    if (fd_read_block(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t)) {
        *id = ID_TYPE_UNDEF;
        return NULL;
    }

    // Read sequence.
    char *seq = (char*) malloc(len + 1);
    if (seq == NULL) {
        *id = ID_TYPE_UNDEF;
        return NULL;
    }
    if (fd_read_block(fd, seq, len) != len) {
        free(seq);
        *id = ID_TYPE_UNDEF;
        return NULL;
    }

    seq[len] = 0;
    return seq;
}

bool send_comp_idx(int fd) {
    // Create 'cumpute index' message header.
    DUP_IO_header header;
    header.command = DUP_IO_COMP_IDX;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    return true;
}

bool send_init_sig(int fd, unsigned int length, bool RNA) {
    if (length < 10)
        return false;

    // Create 'add sequence' message header.
    DUP_IO_header header;
    header.command = DUP_IO_INIT_SIG;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send length.
    if (fd_write(fd, (char*) &length, sizeof(unsigned int))
            != sizeof(unsigned int))
        return false;

    // Send type.
    if (fd_write(fd, (char*) &RNA, sizeof(bool)) != sizeof(bool))
        return false;

    return true;
}

bool recv_init_sig(int fd, unsigned int *length, bool *RNA) {
    // Read length.
    if (fd_read_block(fd, (char*) length, sizeof(unsigned int))
            != sizeof(unsigned int)) {
        *length = 0;
        return false;
    }

    // Read type.
    if (fd_read_block(fd, (char*) RNA, sizeof(bool)) != sizeof(bool)) {
        *length = 0;
        return false;
    }
    return true;
}

bool send_qry_next_sig(int fd) {
    // Create 'cumpute index' message header.
    DUP_IO_header header;
    header.command = DUP_IO_QRY_NEXT_SIG;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    return true;
}

bool send_ans_next_sig(int fd, const char *sig) {
    if (sig == NULL)
        return false;

    // Create 'add sequence' message header.
    DUP_IO_header header;
    header.command = DUP_IO_ANS_NEXT_SIG;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send signature length.
    size_t len = strlen(sig);
    if (fd_write(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return false;

    // Send signature.
    if (fd_write(fd, sig, len) != len)
        return false;

    return true;
}

char *recv_ans_next_sig(int fd) {
    // Read signature length.
    size_t len = 0;
    if (fd_read_block(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return NULL;

    // Read signature.
    char *sig = (char*) malloc(len + 1);
    if (sig == NULL)
        return NULL;

    if (fd_read_block(fd, sig, len) != len) {
        free(sig);
        return NULL;
    }

    sig[len] = 0;
    return sig;
}

bool send_qry_match_sig(int fd, const char *sig, double mm, double mm_dist,
        bool use_wmis) {
    if (sig == NULL)
        return false;

    // Create 'add sequence' message header.
    DUP_IO_header header;
    header.command = DUP_IO_QRY_MATCH_SIG;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send signature length.
    size_t len = strlen(sig);
    if (fd_write(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return false;

    // Send signature.
    if (fd_write(fd, sig, len) != len)
        return false;

    // Send mismatches.
    if (fd_write(fd, (char*) &mm, sizeof(double)) != sizeof(double))
        return false;

    // Send mismatch distance.
    if (fd_write(fd, (char*) &mm_dist, sizeof(double)) != sizeof(double))
        return false;

    // Send use weighted mismatches flag.
    if (fd_write(fd, (char*) &use_wmis, sizeof(bool)) != sizeof(bool))
        return false;

    return true;
}

char *recv_qry_match_sig(int fd, double *mm, double *mm_dist, bool *use_wmis) {
    // Read signature length.
    size_t len = 0;
    if (fd_read_block(fd, (char*) &len, sizeof(size_t)) != sizeof(size_t))
        return NULL;

    // Read signature.
    char *sig = (char*) malloc(len + 1);
    if (sig == NULL)
        return NULL;
    if (fd_read_block(fd, sig, len) != len) {
        free(sig);
        return NULL;
    }

    // Read mismatches.
    if (fd_read_block(fd, (char*) mm, sizeof(double)) != sizeof(double)) {
        free(sig);
        return NULL;
    }
    // Read mismatch distance.
    if (fd_read_block(fd, (char*) mm_dist, sizeof(double)) != sizeof(double)) {
        free(sig);
        return NULL;
    }
    // Send use weighted mismatches flag.
    if (fd_read_block(fd, (char*) use_wmis, sizeof(bool)) != sizeof(bool)) {
        free(sig);
        return NULL;
    }

    sig[len] = 0;
    return sig;
}

bool send_ans_match_sig(int fd, const IntSet *matched_ids,
        unsigned int og_matches) {
    // Create 'matched ids' message header.
    DUP_IO_header header;
    header.command = DUP_IO_ANS_MATCH_SIG;
    header.magic = magic_header;
    header.id = ++ID_counter;

    // Send header.
    if (fd_write(fd, (char*) &header, sizeof(DUP_IO_header))
            != sizeof(DUP_IO_header))
        return false;

    // Send number of outgroup matches.
    if (fd_write(fd, (char*) &og_matches, sizeof(unsigned int))
            != sizeof(unsigned int))
        return false;

    // Send number of IntSet entries.
    unsigned int size = matched_ids->size();
    if (fd_write(fd, (char*) &size, sizeof(unsigned int))
            != sizeof(unsigned int))
        return false;

    // Send matched IDs...
    for (unsigned int i = 0; i < size; ++i) {
        id_type id = matched_ids->val(i);
        if (fd_write(fd, (char*) &id, sizeof(id_type)) != sizeof(id_type))
            return false;
    }

    return true;
}

bool recv_ans_match_sig(int fd, IntSet *&matched_ids,
        unsigned int &og_matches) {
    // Receive number of outgroup matches.
    if (fd_read_block(fd, (char*) &og_matches, sizeof(unsigned int))
            != sizeof(unsigned int))
        return false;

    // Receive number of IntSet entries.
    unsigned int size = 0;
    if (fd_read_block(fd, (char*) &size, sizeof(unsigned int))
            != sizeof(unsigned int))
        return false;

    // Send matched IDs...
    matched_ids->clear();
    for (unsigned int i = 0; i < size; ++i) {
        id_type id = ID_TYPE_UNDEF;
        if (fd_read_block(fd, (char*) &id, sizeof(id_type)) != sizeof(id_type))
            return false;
        matched_ids->add(id);
    }

    return true;
}

