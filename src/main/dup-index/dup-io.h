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

/*
 * DUP-IO commands
 */
enum DUP_IO_command {
    DUP_IO_ERROR = 0, DUP_IO_QUIT, DUP_IO_ECHO
};

int init_fd(int fd);

DUP_IO_command wait_for_command(int fd);

bool send_quit(int fd);

bool send_echo(int fd, const char *message);

char* recv_echo(int fd);

#endif /* DUP_INDEX_IO_H_ */
