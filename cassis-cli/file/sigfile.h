/*!
 * Dump the results from a CaSSiSTree into separate signature files.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
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

#ifndef CASSIS_SIGFILE_H_
#define CASSIS_SIGFILE_H_

#include <cassis/tree.h>
#include <cassis/indexinterface.h>

/*!
 * Dump the results from a CaSSiSTree into multiple CSV tables.
 * This is the 'classic' CaSSiS output.
 *
 * \param tree CaSSiSTree with the processed results.
 * \param iface Search index interface, for detailed match list.
 * \return true, if successful. Otherwise false.
 */
bool dump2Textfiles(CaSSiSTree *tree, IndexInterface *iface);

#endif /* CASSIS_SIGFILE_H_ */
