/*!
 * Generates all signatures of a defined length.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
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

#include "gen-signatures.h"
#include <cstdlib>

/*!
 * Constructor.
 */
GenSignatures::GenSignatures() :
m_counter(0), m_limit(0), m_length(0), m_buffer(NULL) {
    m_nucleotides[0] = 'A';
    m_nucleotides[1] = 'C';
    m_nucleotides[2] = 'G';
    m_nucleotides[3] = 'T';
}

/*!
 * Destructor.
 */
GenSignatures::~GenSignatures() {
    free(m_buffer);
}

/*!
 * Initializes/resets the signature generation.
 * \param length Oligonucleotide length. (Valid range: 1 - 30)
 * \param RNA True for RNA sequences (ACGU), false for DNA (ACGT).
 */
bool GenSignatures::init(unsigned int length, bool RNA) {
    // Signature length has to be in the range 1 - 30.
    if (length == 0 || length > 30)
        return false;

    // Generate RNA signatures if this is requested, otherwise DNA.
    if (RNA)
        m_nucleotides[3] = 'U';
    else
        m_nucleotides[3] = 'T';

    // Reset variables. m_pos is initially undefined (i.e. length+1).
    m_length = length;
    m_counter = 0;
    m_limit = 1;
    m_limit <<= m_length; // Shift two times...
    m_limit <<= m_length;

    // Create the signature buffers.
    free(m_buffer);
    m_buffer = (char *) calloc(m_length + 1, sizeof(char));
    if (!m_buffer)
        return false;

    return true;
}

/*!
 * Generates an oligonucleotide sequence.
 * \return NULL if an error occurred or all signatures were generated.
 */
const char *GenSignatures::next() {
    // Return NULL, if uninitialized. Remove this check if relevant for speed.
    if (m_length == 0)
        return NULL;

    // We have reached our limit, i.e. returned all valid signatures.
    if (m_counter == m_limit)
        return NULL;

    // Fill the buffer with characters and return it...
    uint64_t counter = m_counter++;
    unsigned int length = m_length;
    while (length--) {
        m_buffer[length] = m_nucleotides[counter & 0x03];
        counter >>= 2;
    }
    return m_buffer;
}
