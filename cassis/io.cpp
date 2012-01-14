/*!
 * BGRT File I/O
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2010,2011
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

#include "io.h"

#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <stdint.h>

/*!
 * Reads a fixed-length quantity type from the input stream.
 * Inline code: no error checking.
 */
template<typename T> inline T readType(std::istream &stream) {
    T value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return value;
}

/*!
 * Writes a fixed-length quantity type to the output stream.
 * Inline code: no error checking.
 */
template<typename T> inline void writeType(const T& value,
        std::ostream &stream) {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

/*!
 * Reads an unsigned variable-length quantity integer from the
 * input stream. (VLQ/Base128 VarUInt)
 */
inline uint32_t readVarUInt(std::istream &stream) {
    uint32_t value = 0;
    uint8_t bits = 0;
    do {
        value <<= 7;
        stream.read(reinterpret_cast<char*>(&bits), 1);
        value = value | (bits & 0x7F);
    } while (bits & 0x80);
    return value;
}

/*!
 * Writes an unsigned variable-length quantity integer to the
 * output stream. (VLQ/Base128 VarUInt)
 */
inline void writeVarUInt(uint32_t value, std::ostream &stream) {
    char buffer[5];
    buffer[4] = 0x7F & value;
    int pos = 4;
    while (value > 0x7F) {
        value >>= 7;
        buffer[--pos] = (0x7F & value) | 0x80;
    };
    stream.write(reinterpret_cast<const char*>(buffer + pos), 5 - pos);
}

/*!
 * Read IntSet from stream
 */
IntSet *readIntSet(std::istream &stream) {
    // Read IntSet size...
    unsigned int size = readVarUInt(stream);

    // Create the IntSet...
    IntSet *intset = new IntSet(size);
    intset->setSize(size);

    // Write integer values...
    for (unsigned int i = 0; i < size; ++i)
        intset->set(i, readVarUInt(stream));

    return intset;
}

/*!
 * Write IntSet to stream
 */
bool writeIntSet(const IntSet *intset, std::ostream &stream) {
    if (!intset)
        return false;

    // Write IntSet size...
    writeVarUInt(intset->size(), stream);

    // Write integer values...
    for (unsigned int i = 0; i < intset->size(); ++i)
        writeVarUInt(intset->val(i), stream);

    return true;
}

/*!
 * Read UnorderedIntSet from stream
 */
UnorderedIntSet *readUnorderedIntSet(std::istream &stream) {
    // Read IntSet size...
    unsigned int size = readVarUInt(stream);

    // Create the UnorderedIntSet...
    UnorderedIntSet *uintset = new UnorderedIntSet(size);
    uintset->setSize(size);

    // Write integer values...
    for (unsigned int i = 0; i < size; ++i)
        uintset->set(i, readVarUInt(stream));

    return uintset;
}

/*!
 * Write IntSet to stream
 */
bool writeUnorderedIntSet(const UnorderedIntSet *uintset,
        std::ostream &stream) {
    if (!uintset)
        return false;

    // Write IntSet size...
    writeVarUInt(uintset->size(), stream);

    // Write integer values...
    for (unsigned int i = 0; i < uintset->size(); ++i)
        writeVarUInt(uintset->val(i), stream);

    return true;
}

/*!
 * Read a string (char *) from the stream
 */
inline char *readString(std::istream &stream) {
    // Read string length...
    unsigned int len = readVarUInt(stream);

    // Read and append string to StrSet...
    char *str = (char*) malloc(len + 1);
    if (len)
        stream.read(str, len);
    str[len] = 0x00;

    return str;
}

/*!
 * Write a string to the stream
 */
inline void writeString(const char *str, std::ostream &stream) {
    if (str) {
        // Write string length...
        unsigned int len = strlen(str);
        writeVarUInt(len, stream);

        // Write string...
        if (len)
            stream.write(str, len);
    } else {
        // NULL strings are handled as empty strings
        unsigned int len = 0;
        writeVarUInt(len, stream);
    }
}

/*!
 * Read StrSet from stream
 */
StrSet *readStrSet(std::istream &stream) {
    // Read StrSet size...
    unsigned int size = readVarUInt(stream);

    // Create the IntSet...
    StrSet *strset = new StrSet(size);
    strset->setSize(size);

    // Read string values...
    for (unsigned int i = 0; i < size; ++i)
        strset->set(i, readString(stream));

    return strset;
}

/*!
 * Write StrSet to stream
 */
bool writeStrSet(const StrSet *strset, std::ostream &stream) {
    if (!strset)
        return false;

    // Write StrSet size to stream.
    writeVarUInt(strset->size(), stream);

    // Write string values to stream.
    for (unsigned int i = 0; i < strset->size(); ++i)
        writeString(strset->val(i), stream);

    return true;
}

/*!
 * Write a Base4_t to stream
 */
inline Base4 *readBase4_t(std::istream &stream) {
    // Read uncompressed string length in bytes.
    unsigned short len = readVarUInt(stream);

    // Calculate of base4-encoded chars (2 bits/char).
    unsigned short base4_len = len / 4;
    if (len % 4)
        ++base4_len;

    // Read and append string to StrSet...
    char *str = (char*) malloc(base4_len);
    if (base4_len)
        stream.read(str, base4_len);

    Base4 *base4 = new Base4();
    base4->set(str, len);
    return base4;
}

/*!
 * Write a Base4_t to stream
 */
inline void writeBase4_t(const Base4 *base4, std::ostream &stream) {
    // Calculate the number of base4-encoded chars (2 bits/char).
    unsigned short base4_len = base4->len() / 4;
    if (base4->len() % 4)
        ++base4_len;

    // Write the uncompressed string length in bytes.
    writeVarUInt(base4->len(), stream);

    // Write base4-encoded string
    if (base4_len > 0)
        stream.write(base4->seq(), base4_len);
}

/*!
 * Read Base4Set from stream
 */
Base4Set *readBase4Set(std::istream &stream) {
    // Read Base4Set size...
    unsigned int size = readVarUInt(stream);

    // Create the Base4Set...
    Base4Set *set = new Base4Set(size);
    set->setSize(size);

    // Read string values...
    for (unsigned int i = 0; i < size; ++i) {
        set->set(i, readBase4_t(stream));
    }

    return set;
}

/*!
 * Write StrSet to stream
 */
bool WriteBase4Set(const Base4Set *set, std::ostream &stream) {
    if (!set)
        return false;

    // Write Base4Set size to stream.
    writeVarUInt(set->size(), stream);

    // Write Base4_t values to stream.
    for (unsigned int i = 0; i < set->size(); ++i)
        writeBase4_t(set->val(i), stream);

    return true;
}

/*!
 * Write a BGRT node to an output stream.
 */
BgrTreeNode *readBGRTEntry(std::istream &stream, bool base4_compressed) {
    // Create new node...
    BgrTreeNode *node = (BgrTreeNode *) calloc(1, sizeof(struct BgrTreeNode));

    // Read: IntSet *species:
    node->species = readIntSet(stream);

    // Read: UnorderedIntSet *supposed_outgroup_matches;
    node->supposed_outgroup_matches = readUnorderedIntSet(stream);

    // Write: StrSet *signatures:
    if (base4_compressed)
        node->signatures.base4 = readBase4Set(stream);
    else
        node->signatures.str = readStrSet(stream);

    // Count child nodes and write the result to the stream...
    uint16_t num_children = (uint16_t) readVarUInt(stream);

    // Read children...
    if (num_children--) {
        BgrTreeNode *child = readBGRTEntry(stream, base4_compressed);
        node->children = child;
        child->parent = node;
        while (num_children) {
            BgrTreeNode *next = readBGRTEntry(stream, base4_compressed);
            next->parent = node;
            child->next = next;
            child = next;
            --num_children;
        }
    }

    return node;
}

/*!
 * Write a BGRT node to an output stream.
 */
bool writeBGRTEntry(BgrTreeNode *node, std::ostream &stream,
        bool base4_compressed) {
    if (!node)
        return false;

    // Write: IntSet *species:
    writeIntSet(node->species, stream);

    // Write: UnorderedIntSet *supposed_outgroup_matches;
    writeUnorderedIntSet(node->supposed_outgroup_matches, stream);

    // Write: StrSet *signatures:
    if (base4_compressed)
        WriteBase4Set(node->signatures.base4, stream);
    else
        writeStrSet(node->signatures.str, stream);

    // Count child nodes and write the result to the stream...
    uint16_t num_children = 0;
    BgrTreeNode *child = node->children;
    while (child) {
        ++num_children;
        child = child->next;
    }
    writeVarUInt(num_children, stream);

    // Write children...
    child = node->children;
    while (child) {
        writeBGRTEntry(child, stream, base4_compressed);
        child = child->next;
    }
    return true;
}

/*!
 * Read a BGRT header from an input stream
 */
BgrTree *readBGRTHeader(std::istream &stream) {
    // Read the BGRT encoding type.
    bool base4_compressed = (bool) readVarUInt(stream);

    // BGRTree: read first level depth.
    unsigned int num_species = (uint32_t) readVarUInt(stream);

    // Create the tree.
    BgrTree *bgrt = BgrTree_create(num_species, base4_compressed);

    // Number of ingroup mismatches, that were used to compute the BGRTree.
    bgrt->ingroup_mismatch_distance = (uint16_t) readVarUInt(stream);

    // Mismatch distance that was used to compute the BGRTree.
    bgrt->outgroup_mismatch_distance = (uint16_t) readVarUInt(stream);

    // Minimum and maximum oligonucleotide length in the BGRTree.
    bgrt->min_oligo_len = (uint16_t) readVarUInt(stream);
    bgrt->max_oligo_len = (uint16_t) readVarUInt(stream);

    // Minimum and maximum G+C content used to filter the BGRTree.
    // Round two digits after the comma.
    bgrt->min_gc = readType<float>(stream);
    bgrt->max_gc = readType<float>(stream);

    // Minimum and maximum melting temp. used to filter the BGRTree.
    // Round two digits after the comma.
    bgrt->min_temp = readType<float>(stream);
    bgrt->max_temp = readType<float>(stream);

    // A comment that can be added to the BGRTree file.
    bgrt->comment = readString(stream);

    return bgrt;
}

/*!
 * Read a BGRT from an input stream.
 */
BgrTree *readBGRTStream(NameMap *map, std::istream &stream) {
    // Read the BGRTheader from stream and fetch the newly created BGRTree.
    BgrTree *bgrt = readBGRTHeader(stream);
    if (!bgrt)
        return NULL;

    // Read NameMap size
    map->clear();
    uint32_t map_size = (uint32_t) readVarUInt(stream);
    map->setSize(map_size);

    // Read the name strings from the buffer
    for (uint32_t id = 0; id < map_size; ++id) {
        char *name = readString(stream);
        map->set(id, name);
        free(name);
    }

    // BGRTree: traverse through 1st level...
    for (uint32_t i = 0; i < bgrt->num_species; ++i) {
        // Fetch child nodes at position 'i'.
        uint16_t num_children = (uint16_t) readVarUInt(stream);

        // Read the child nodes from the stream...
        if (num_children)
            bgrt->nodes[i] = readBGRTEntry(stream, bgrt->base4_compressed);
    }

    return bgrt;
}

/*!
 * Adler32 checksum (from an input stream).
 * Computation starts at the current(!) seekg-position.
 *
 * \param stream Input stream
 * \return Adler32 checksum
 */
uint32_t adler32(std::istream &stream) {
    uint32_t s1 = 1;
    uint32_t s2 = 0;
    uint8_t c = stream.get();
    while (stream.good()) {
        s1 = (s1 + c) % 65521;
        s2 = (s2 + s1) % 65521;
        c = stream.get();
    }
    return (s2 << 16) | s1;
}

/*!
 * BGRT File identifier
 * [0-3]  == 'BGRT'
 * [4]    == bgrt_file_version
 * [5-7]  == 0x00 (reserved)
 */
const char BGRT_FILE_ID[8] = { 0x42, 0x47, 0x52, 0x54, 0x02, 0x00, 0x00, 0x00 };

/*!
 * Read a BGRT from a file.
 */
BgrTree *readBGRTFile(NameMap *map, const char *filename) {
    // Open input file stream...
    std::ifstream file;
    file.open(filename, std::ios::in | std::ios::binary);
    if (file.bad())
        return NULL;

    // Read the BGRT file identifier.
    char header[8];
    file.read(header, 8);
    // Compare first 8 bytes, i.e. "do we have a BGRT file?"
    if (strncmp(BGRT_FILE_ID, header, 8) != 0)
        return NULL;

    // Fetch the stored checksum...
    uint32_t stored_checksum = readType<uint32_t>(file);
    // ...and generate a file checksum from byte 12 on.
    uint32_t checksum = adler32(file);

    // If the checksums do not match, the file is possibly corrupted.
    if (checksum != stored_checksum) {
        file.close();
        return NULL;
    }

    // Reset the read position and fetch the BGRT from the file.
    file.clear();
    file.seekg(12, std::ios_base::beg);
    BgrTree *bgrt = readBGRTStream(map, file);
    file.close();
    return bgrt;
}

/*!
 * Write a BGRT header to an output stream
 */
void writeBGRTHeader(BgrTree *bgrt, std::ostream &stream) {
    // Write the BGRT encoding type
    writeVarUInt(bgrt->base4_compressed, stream);

    // Write first level depth.
    writeVarUInt(bgrt->num_species, stream);

    // Number of ingroup mismatches, that were used to compute the BGRTree.
    writeVarUInt(bgrt->ingroup_mismatch_distance, stream);

    // Mismatch distance that was used to compute the BGRTree.
    writeVarUInt(bgrt->outgroup_mismatch_distance, stream);

    // Minimum and maximum oligonucleotide length in the BGRTree.
    writeVarUInt(bgrt->min_oligo_len, stream);
    writeVarUInt(bgrt->max_oligo_len, stream);

    // Minimum and maximum G+C content used to filter the BGRTree.
    // Round two digits after the comma.
    writeType<float>(bgrt->min_gc, stream);
    writeType<float>(bgrt->max_gc, stream);

    // Minimum and maximum melting temp. used to filter the BGRTree.
    // Round two digits after the comma.
    writeType<float>(bgrt->min_temp, stream);
    writeType<float>(bgrt->max_temp, stream);

    // A comment that can be added to the BGRTree file.
    writeString(bgrt->comment, stream);
}

/*!
 * Write a BGRT to an output stream.
 */
bool writeBGRTStream(BgrTree *bgrt, NameMap *map, std::ostream &stream) {
    assert(bgrt);

    // Write bgrt file header
    writeBGRTHeader(bgrt, stream);

    // Write NameMap size (4 bytes)
    writeVarUInt(map->size(), stream);

    // Write NameMap
    for (uint32_t id = 0; id < map->size(); ++id) {
        // Fetch the name, and write it to the stream.
        std::string name = map->name(id);
        writeString(name.c_str(), stream);
    }

    // BGRTree: traverse through 1st level...
    for (uint32_t i = 0; i < bgrt->num_species; ++i) {
        // Count child nodes at position 'i'
        // and write the result to the stream...
        uint16_t num_children = 0;
        BgrTreeNode *child = bgrt->nodes[i];
        while (child) {
            ++num_children;
            child = child->next;
        }
        writeVarUInt(num_children, stream);

        // Write the nodes to the stream...
        if (num_children)
            writeBGRTEntry(bgrt->nodes[i], stream, bgrt->base4_compressed);
    }
    return true;
}

/*!
 * Write a BGRT to a file.
 */
bool writeBGRTFile(BgrTree *bgrt, NameMap *map, const char *filename) {
    // Open I/O file stream and write the BGRT into it...
    std::fstream file;
    file.open(filename,
            std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (file.bad()) {
        // Something went wrong...
        return false;
    }

    // Write file identifier (8 bytes).
    file.write(BGRT_FILE_ID, 8);

    // Add 4 bytes as a wildcard for the checksum.
    uint32_t checksum = 0;
    writeType<uint32_t>(checksum, file);

    // Add the BGRT file...
    bool retval = writeBGRTStream(bgrt, map, file);

    // Reset the get position pointer to byte #12 and generate a file checksum.
    file.clear();
    file.seekg(12, std::ios_base::beg);
    checksum = adler32(file);

    // Set the put position pointer to byte #8 and write the checksum.
    file.clear();
    file.seekp(8, std::ios_base::beg);
    writeType<uint32_t>(checksum, file);

    // Close and exit.
    file.close();
    return retval;
}
