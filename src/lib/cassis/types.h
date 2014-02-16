/*!
 * Basic BGRT types
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2009-2012
 *     Kai Christian Bader <mail@kaibader.de>
 *     Christian Grothoff <christian@grothoff.org>
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

#ifndef BGRT_TYPE_H_
#define BGRT_TYPE_H_

#include <cstring>
#include <cstdlib>

/*!
 * IDs should be of a defined type throughout CaSSiS:
 */
typedef unsigned int id_type;

/*!
 * This value is used to mark undefined values/nodes/...
 */
#ifndef ID_TYPE_UNDEF
#define ID_TYPE_UNDEF ((id_type)-1)
#endif

/*!
 * Ordered set template class.
 */
template<typename T> class OSet {
public:
    /*!
     * Constructor.
     * \param initial_size Size, that should initially be allocated.
     */
    OSet(unsigned int initial_size = 0);

    /*!
     * Destructor
     */
    virtual ~OSet();

    /*!
     * Compares two sets.
     * \return True, if equal. Otherwise false.
     */
    bool equals(const OSet<T> *oset);

    /*!
     * Tests, if the given set is a subset of oset.
     * \return True, if it is a subset. Otherwise false.
     */
    bool isSubsetOf(const OSet<T> *oset);

    /*!
     * Adds a value/element v to the set.
     * The set takes control over the element (i.e. will de-allocate it).
     * \return Position where the element was inserted.
     */
    unsigned int add(const T &v);

    /*!
     * Get value
     * \return Value at pos in the set.
     */
    const T val(unsigned int pos) const;

    /*!
     * Sets the value at position pos in the set to v.
     * \return
     */
    void set(unsigned int pos, const T &v);

    /*!
     * Sets the number of valid elements to s.
     * handle with care!
     */
    void setSize(unsigned int s);

    /*!
     * Set size.
     * \return Number of stored elements in the set.
     */
    unsigned int size() const;

    /*!
     * Clears the set. All entries are de-allocated (deleted).
     */
    void clear();

    /*!
     * Clears the set without de-allocating (deleting) the entries.
     */
    void unref();

    /*!
     * Returns a pointer to the stored values of the set.
     * Handle with care!
     */
    const T *val_ptr();
protected:
    /*!
     * Number of entries in the set.
     */
    unsigned int m_size;

    /*!
     * Number of entries for which we have allocated space.
     */
    unsigned int m_vsize;

    /*!
     * Values in the set.
     */
    T *m_val;
private:
    /*!
     * Copy constructor.
     * Not implemented --> private.
     */
    OSet(const OSet<T>&);

    /*!
     * Assignment operator.
     * Not implemented. --> private.
     */
    OSet<T> &operator=(const OSet<T>&);
};

/*!
 * Constructor.
 * \param initial_size Size, that should initially be allocated.
 */
template<typename T> OSet<T>::OSet(unsigned int initial_size) :
        m_size(0), m_vsize(0), m_val(0) {
    if (initial_size > 0) {
        this->m_val = (T*) calloc(initial_size, sizeof(T));
        this->m_vsize = initial_size;
    }
}

/*!
 * Destructor
 */
template<typename T> OSet<T>::~OSet() {
    clear();
    free(this->m_val);
    this->m_val = NULL;
    this->m_vsize = 0;
}

/*!
 * Compares two sets. Utilizes memcmp(), might be cheesy...
 * \return True, if equal. Otherwise false.
 */
template<typename T> bool OSet<T>::equals(const OSet<T> *oset) {
    return (this->m_size == oset->m_size)
            && (memcmp(this->m_val, oset->m_val, this->m_size * sizeof(T)));
}

/*!
 * Tests, if the given set is a subset of val.
 * \return True, if it is a subset. Otherwise false.
 */
template<typename T> bool OSet<T>::isSubsetOf(const OSet<T> *oset) {
    if (this->m_size > oset->m_size)
        return false; // This set has more entries than val -> not a subset.

    unsigned int o1 = 0;
    unsigned int o2 = 0;
    // Checks, if integer set2 is a subset of integer set1
    while ((o1 < oset->m_size) && (o2 < this->m_size)) {
        if (oset->m_val[o1] > this->m_val[o2])
            return false; // Entry does not exist in vals -> not a subset.
        if (oset->m_val[o1] == this->m_val[o2])
            o2++;
        o1++;
    }
    return (o2 == this->m_size);
    // Return 'true, if all entries were found.
}

/*!
 * Adds an element val to the set.
 * The elements are sorted in ascending order.
 * The set takes control over the element (i.e. will de-allocate it).
 * \return Position where the element was inserted.
 */
template<typename T> unsigned int OSet<T>::add(const T &v) {
    // Binary search for the insert position of val.
    // (Only done, if there are already entries in the set.)
    unsigned int i = 0, mid = 0, max = this->m_size;
    while (i < max) {
        mid = i + ((max - i) / 2);
        if (v > this->m_val[mid])
            i = mid + 1;
        else
            max = mid;
    }

    // Return, if already in our list.
    if ((i < this->m_size) && (this->m_val[i] == v))
        return i;

    if (this->m_size < this->m_vsize) {
        memmove(&this->m_val[i + 1], &this->m_val[i],
                (this->m_size - i) * sizeof(unsigned int));
        this->m_val[i] = v;
        this->m_size++;
        return i;
    }
    /* need to grow */
    if (this->m_vsize >= 512)
        this->m_vsize += 512;
    else
        this->m_vsize *= 2;
    if (this->m_vsize == 0)
        this->m_vsize = 2;
    unsigned int *nval = (unsigned int *) malloc(
            sizeof(unsigned int) * this->m_vsize);
    memcpy(nval, this->m_val, i * sizeof(unsigned int));
    nval[i] = v;
    memcpy(&nval[i + 1], &this->m_val[i],
            (this->m_size - i) * sizeof(unsigned int));
    free(this->m_val);
    this->m_val = nval;
    this->m_size++;
    return i;
}

/*!
 * Get value
 * \return Value at pos in the set.
 */
template<typename T> const T OSet<T>::val(unsigned int pos) const {
    // Error checking (pos valid?) not done...
    return this->m_val[pos];
}

/*!
 * Set the value at Position pos in the set.
 */
template<typename T> void OSet<T>::set(unsigned int pos, const T &value) {
    if (pos < m_vsize)
        m_val[pos] = value;
}

/*!
 * Sets the number of valid elements.
 * handle with care!
 */
template<typename T> void OSet<T>::setSize(unsigned int s) {
    if (s <= m_vsize)
        m_size = s;
}

/*!
 * Set size.
 * \return Number of stored elements in the set.
 */
template<typename T> unsigned int OSet<T>::size() const {
    return this->m_size;
}

/*!
 * Clears a set of 'unsigned int'.
 * Allocated memory (this->m_vals) is kept.
 */
template<> inline void OSet<unsigned int>::clear() {
    this->m_size = 0;
}

/*!
 * Clears a set of 'char*' strings.
 * Allocated memory (this->m_vals) is kept.
 */
template<> inline void OSet<char*>::clear() {
    for (unsigned int i = 0; i < this->m_size; ++i)
        free(this->m_val[i]);
    this->m_size = 0;
}

/*!
 * Clears the set. All entries are de-allocated (deleted).
 * Allocated memory (this->m_vals) is kept.
 */
template<typename T> void OSet<T>::clear() {
    for (unsigned int i = 0; i < this->m_size; ++i)
        delete this->m_val[i];
    this->m_size = 0;
}

/*!
 * Clears the set without de-allocating (deleting) the entries.
 * Allocated memory (this->m_vals) is kept.
 */
template<typename T> void OSet<T>::unref() {
    this->m_size = 0;
}

/*!
 * Returns a pointer to the stored values of the set.
 * Handle with care!
 */
template<typename T> const T *OSet<T>::val_ptr() {
    return this->m_val;
}

/*!
 * Unordered set template class.
 * Most of its functionality is identical to the ordered set.
 */
template<typename T> class USet: public OSet<T> {
public:
    /*!
     * Constructor.
     * \param initial_size Size, that should initially be allocated.
     */
    USet(unsigned int initial_size = 0);

    /*!
     * Adds an element val to the set.
     * The elements are sorted in ascending order.
     * The set takes control over the element (i.e. will de-allocate it).
     * \return Position where the element was inserted.
     */
    unsigned int add(const T &val);
};

/*!
 * Constructor.
 * \param initial_size Size, that should initially be allocated.
 */
template<typename T> USet<T>::USet(unsigned int initial_size) :
        OSet<T>::OSet(initial_size) {
}

/*!
 * Adds a value/element v to the set.
 * The set takes control over the element (i.e. will de-allocate it).
 * \return Position where the element was inserted.
 */
template<typename T> unsigned int USet<T>::add(const T &v) {
    if (this->m_vsize == this->m_size) {
        if (this->m_vsize >= 512)
            this->m_vsize += 512;
        else
            this->m_vsize *= 2;
        if (this->m_vsize == 0)
            this->m_vsize = 2;
        this->m_val = (T*) realloc(this->m_val, sizeof(T) * this->m_vsize);
    }
    this->m_val[this->m_size] = v;

    // Should return the position where the element was inserted. Untested.
    return this->m_size++;
}

/*!
 * Unordered reference set template class.
 * Most of its functionality is identical to the unordered set.
 *
 * It does not take control over its elements.
 * -->No delete/free is done on the elements.
 */
template<typename T> class URefSet: public USet<T> {
public:
    /*!
     * Constructor.
     * \param initial_size Size, that should initially be allocated.
     */
    URefSet(unsigned int initial_size = 0);

    /*!
     * Destructor
     */
    virtual ~URefSet();

    /*!
     * Clears the set. No entries are de-allocated!
     */
    void clear();
};

/*!
 * Constructor.
 * \param initial_size Size, that should initially be allocated.
 */
template<typename T> URefSet<T>::URefSet(unsigned int initial_size) :
        USet<T>::USet(initial_size) {
}

/*!
 * Destructor
 */
template<typename T> URefSet<T>::~URefSet() {
    clear();
}

/*!
 * Clears the set. No entries are de-allocated!
 */
template<typename T> inline void URefSet<T>::clear() {
    this->m_size = 0;
    free(this->m_val);
    this->m_val = NULL;
    this->m_vsize = 0;
}

/*!
 * Set of strings.
 */
typedef USet<char*> StrSet;

/*!
 * Set of reference to strings.
 */
typedef URefSet<char*> StrRefSet;

/*!
 * Unordered set of integers.
 */
typedef USet<unsigned int> UnorderedIntSet;

/*!
 * Ordered set of integers.
 */
typedef OSet<unsigned int> IntSet;

#endif /* BGRT_TYPE_H_ */
