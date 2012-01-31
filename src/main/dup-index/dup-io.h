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

#ifndef DUP_INDEX_IO_H_
#define DUP_INDEX_IO_H_

#include <cassis/types.h>

/*
 * DUP-IO commands
 */
enum DUP_IO_command {
    DUP_IO_ERROR = 0,
    DUP_IO_QUIT,
    DUP_IO_ECHO,
    DUP_IO_SEQ,
    DUP_IO_COMP_IDX,
    DUP_IO_INIT_SIG,
    DUP_IO_QRY_NEXT_SIG,
    DUP_IO_ANS_NEXT_SIG,
    DUP_IO_QRY_MATCH_SIG,
    DUP_IO_ANS_MATCH_SIG
};

int init_fd(int fd);

DUP_IO_command wait_for_command(int fd);

bool send_quit(int fd);

bool send_echo(int fd, const char *message);

char* recv_echo(int fd);

bool send_seq(int fd, id_type id, const char *seq);

char *recv_seq(int fd, id_type *id);

bool send_comp_idx(int fd);

bool send_init_sig(int fd, unsigned int length, bool RNA);

bool recv_init_sig(int fd, unsigned int *length, bool *RNA);

bool send_qry_next_sig(int fd);

bool send_ans_next_sig(int fd, const char *sig);

char *recv_ans_next_sig(int fd);

bool send_qry_match_sig(int fd, const char *signature, double mm,
        double mm_dist, bool use_wmis);

char *recv_qry_match_sig(int fd, double *mm, double *mm_dist, bool *use_wmis);

bool send_ans_match_sig(int fd, const IntSet *matched_ids,
        unsigned int og_matches);

bool recv_ans_match_sig(int fd, IntSet *&matched_ids, unsigned int &og_matches);

#endif /* DUP_INDEX_IO_H_ */
