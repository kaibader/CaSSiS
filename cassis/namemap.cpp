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

#include "namemap.h"

#include <string>
#include <map>

/*!
 * NameMap member variables are stored in this class.
 * Avoids compile time dependencies.
 */
class NameMap_priv {
public:
    NameMap_priv() :
        counter(0), name2id_map(), id2name_map() {
    }
    virtual ~NameMap_priv() {
    }
    unsigned int counter;
    std::map<std::string, id_type> name2id_map;
    std::map<id_type, std::string> id2name_map;
};

/*!
 * Name Map (Name <-> ID) -- Constructor
 */
NameMap::NameMap() :
                priv(new NameMap_priv) {
}

/*!
 * Copy constructor.
 */
NameMap::NameMap(const NameMap &map) {
    copy(map);
}

/*!
 * Assignment operator.
 */
NameMap& NameMap::operator=(const NameMap &map) {
    copy(map);
    return *this;
}

/*!
 * Name Map (Name <-> ID) -- Destructor
 */
NameMap::~NameMap() {
    clear();
    delete priv;
}

/*!
 * Copy an existing mapping (all strings are copied.)
 */
void NameMap::copy(const NameMap &map) {
    clear();
    priv->name2id_map.insert(map.priv->name2id_map.begin(),
            map.priv->name2id_map.end());
    priv->id2name_map.insert(map.priv->id2name_map.begin(),
            map.priv->id2name_map.end());
    priv->counter = map.priv->counter;
}

/*!
 * Name Map (Name <-> ID)
 * Inserts/sets an element with the given settings.
 * The element counter is _not_ increased by this function!
 *
 * \param id Sequence identifier
 * \param name Sequence name
 */
void NameMap::set(const id_type new_id, const std::string new_name) {
    // Create/set the entry with the given parameters...
    priv->name2id_map[new_name] = new_id;
    priv->id2name_map[new_id] = new_name;
}

/*!
 * Name Map (Name <-> ID)
 * Appends a name to the map
 *
 * \param name Sequence name
 * \return ID of the sequence name.
 */
id_type NameMap::append(const std::string new_name) {
    // Ignore empty names.
    if (new_name.length() == 0)
        return ID_TYPE_UNDEF;

    // Lets see, if we already have the element in our list.
    std::map<std::string, id_type>::const_iterator it = priv->name2id_map.find(
            new_name);
    // If so, return the stored ID.
    if (it != priv->name2id_map.end())
        return it->second;

    // Create a new lookup-entry...
    unsigned int new_id = priv->counter++;
    priv->name2id_map[new_name] = new_id;
    priv->id2name_map[new_id] = new_name;
    return new_id;
}

std::string NameMap::name(const id_type search_id) const {
    // Return empty string if id is ID_TYPE_UNDEF (== no id).
    if (search_id == ID_TYPE_UNDEF) {
        return std::string();
    }

    // Lets see, if we already have the element in our list.
    std::map<id_type, std::string>::const_iterator it = priv->id2name_map.find(
            search_id);
    if (it != priv->id2name_map.end())
        return it->second;
    return std::string();
}

id_type NameMap::id(const std::string &search_name) const {
    // Lets see, if we already have the element in our list.
    std::map<std::string, id_type>::const_iterator it = priv->name2id_map.find(
            search_name);
    // If so, return the stored ID.
    if (it != priv->name2id_map.end())
        return it->second;

    // Return ID_TYPE_UNDEF (== not an id)
    return ID_TYPE_UNDEF;
}

/*!
 * Caution -- only use this if you know what you are doing.
 *
 * \param size Number of elements in the map
 */
void NameMap::setSize(unsigned int setsize) {
    priv->counter = setsize;
}

unsigned int NameMap::size() const {
    return priv->name2id_map.size();
}

void NameMap::clear() {
    priv->counter = 0;
    priv->name2id_map.clear();
    priv->id2name_map.clear();
}
