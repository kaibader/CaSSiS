/*!
 * BGRT File I/O
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2010-2012
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

#ifndef BGRT_IO_H_
#define BGRT_IO_H_

#include "bgrt.h"
#include "namemap.h"

#include <iostream>

/*!
 * Read a BGRT from an input stream.
 */
BgrTree *readBGRTStream(NameMap *map, std::istream &stream);

/*!
 * Read a BGRT from a file.
 */
BgrTree *readBGRTFile(NameMap *map, const char *filename);

/*!
 * Write a BGRT to an output stream.
 */
bool writeBGRTStream(BgrTree *bgrt, NameMap *map, std::ostream &stream);

/*!
 * Write a BGRT to a file.
 */
bool writeBGRTFile(BgrTree *bgrt, NameMap *map, const char *filename);

#endif /* BGRT_IO_H_ */
