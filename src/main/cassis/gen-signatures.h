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

#ifndef CASSIS_GEN_SIGNATURES_H_
#define CASSIS_GEN_SIGNATURES_H_

#include <stdint.h>

class GenSignatures {
public:
    /*!
     * Constructor.
     */
    GenSignatures();

    /*!
     * Destructor.
     */
    virtual ~GenSignatures();

    /*!
     * Initializes/resets the signature generation.
     * \param length Oligonucleotide length.
     * \param RNA True for RNA sequences (ACGU), false for DNA (ACGT).
     * \return true, if successfully initialized.
     */
    bool init(unsigned int length, bool RNA);

    /*!
     * Generates an oligonucleotide sequence.
     * \return NULL if an error occurred or all signatures were generated.
     */
    const char *next();
private:
    uint64_t m_counter;
    uint64_t m_limit;
    unsigned int m_length;
    char m_nucleotides[4];
    char *m_buffer;
};

#endif /* CASSIS_GEN_SIGNATURES_H_ */
