/*!
 * BGRT merge functionality
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2011,2012
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

#ifndef MERGE_H_
#define MERGE_H_

#include <cassis/bgrt.h>
#include <cassis/namemap.h>

///*!
// * Integrates all BGRT nodes from the 'src' tree into the 'dest' tree.
// * This assumes that all identifiers from src and dest are identical and
// * src #identifiers < dest #identifiers.
// *
// * \param dest_bgrt Destination BGRT
// * \param src_bgrt Source BGRT
// * \return Number of copied nodes. Zero if an error occurred.
// */
//unsigned int BgrTree_merge(struct BgrTree *dest_bgrt,
//        const struct BgrTree *src_bgrt);

/*!
 * Integrates all BGRT nodes from the 'src' tree into the 'dest' tree.
 *
 * \param dest_bgrt Destination BGRT
 * \param dest_map Destination NameMap
 * \param src_bgrt Source BGRT
 * \param src_map Source NameMap
 * \return Number of copied nodes. Zero if an error occurred.
 *
 */
unsigned int BgrTree_merge(struct BgrTree *dest_bgrt, NameMap &dest_map,
        const struct BgrTree *src_bgrt, const NameMap &src_map);

#endif /* MERGE_H_ */
