/*!
 * ID/Name mapping class
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

#ifndef BGRT_NAMEMAP_H_
#define BGRT_NAMEMAP_H_

#include <string>
#include "types.h"

/*!
 * This value is used to mark undefined values/nodes/...
 */
#ifndef ID_TYPE_UNDEF
#define ID_TYPE_UNDEF ((id_type)-1)
#endif

/*!
 * NameMap member variables are stored in here.
 * Avoids compile time dependencies.
 */
class NameMap_priv;

/*!
 * Name Map (Name <-> ID)
 * Allows sequence name to sequence id mapping and lookup.
 */
class NameMap {
public:
    NameMap();
    NameMap(const NameMap &map);
    NameMap& operator=(const NameMap &map);
    virtual ~NameMap();
    void set(const id_type id, const std::string name);
    unsigned int append(const std::string name);
    std::string name(const id_type id) const;
    id_type id(const std::string &name) const;
    void setSize(unsigned int size);
    unsigned int size() const;
    void clear();
private:
    void copy(const NameMap &map);
    NameMap_priv *priv;
};

#endif /* BGRT_NAMEMAP_H_ */
