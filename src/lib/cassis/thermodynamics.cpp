/*!
 * Signature thermodynamics
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

#include "thermodynamics.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

/*!
 * Enthalpy values (--> delta_h; in kcal/mol)
 * (needed for base stacking)
 *
 * AA,AC,AG,AT
 * CA,CC,CG,CT
 * GA,GC,GG,GT
 * TA,TC,TG,TT
 */
//const double Thermodynamics::m_array_h[4][4] = { { -7.9, -8.4, -7.8, -7.2 }, {
//        -8.5, -8.0, -10.6, -7.8 }, { -8.2, -10.6, -8.0, -8.4 }, { -7.2, -8.2,
//        -8.5, -7.9 } };
/*!
 * Entropy values (--> delta_s; in cal/(mol * K))
 * (needed for base stacking)
 *
 * AA,AC,AG,AT
 * CA,CC,CG,CT
 * GA,GC,GG,GT
 * TA,TC,TG,TT
 */
//const double Thermodynamics::m_array_s[4][4] = {
//        { -22.2, -22.4, -21.0, -20.4 }, { -22.7, -19.9, -27.2, -21.0 }, {
//                -22.2, -27.2, -19.9, -22.4 }, { -21.3, -22.2, -22.7, -22.2 } };
/*!
 * Thermodynamics -- Constructor
 */
Thermodynamics::Thermodynamics() :
m_test_gc(false), m_test_tm(false), m_test_tm_basic(false), m_min_gc(0), m_max_gc(
        0), m_gc(0), m_tm_basic(0), m_min_tm(0), m_max_tm(0), m_delta_h(
                0), m_delta_s(0), m_tm(0), m_translation_table(NULL), m_buf_size(
                        0), m_buf_vsize(0), m_buf(NULL) {
    // Initialize translation table (bases --> array indices)
    m_translation_table = (unsigned char *) malloc(256 * sizeof(char));
    memset(m_translation_table, 0xFF, 256);
    m_translation_table['A'] = 0x00;
    m_translation_table['a'] = 0x00;
    m_translation_table['C'] = 0x01;
    m_translation_table['c'] = 0x01;
    m_translation_table['G'] = 0x02;
    m_translation_table['g'] = 0x02;
    m_translation_table['T'] = 0x03;
    m_translation_table['t'] = 0x03;
    m_translation_table['U'] = 0x03;
    m_translation_table['u'] = 0x03;

    // Enthalpy values (--> delta_h; in kcal/mol)
    m_array_h[0][0] = -7.9; // AA
    m_array_h[0][1] = -8.4; // AC
    m_array_h[0][2] = -7.8; // AG
    m_array_h[0][3] = -7.2; // AT
    m_array_h[1][0] = -8.5; // CA
    m_array_h[1][1] = -8.0; // CC
    m_array_h[1][2] = -10.6; // CG
    m_array_h[1][3] = -7.8; // CT
    m_array_h[2][0] = -8.2; // GA
    m_array_h[2][1] = -10.6; // GC
    m_array_h[2][2] = -8.0; // GG
    m_array_h[2][3] = -8.4; // GT
    m_array_h[3][0] = -7.2; // TA
    m_array_h[3][1] = -8.2; // TC
    m_array_h[3][2] = -8.5; // TG
    m_array_h[3][3] = -7.9; // TT

    // Entropy values (--> delta_s; in cal/(mol * K))
    m_array_s[0][0] = -22.2; // AA
    m_array_s[0][1] = -22.4; // AC
    m_array_s[0][2] = -21.0; // AG
    m_array_s[0][3] = -20.4; // AT
    m_array_s[1][0] = -22.7; // CA
    m_array_s[1][1] = -19.9; // CC
    m_array_s[1][2] = -27.2; // CG
    m_array_s[1][3] = -21.0; // CT
    m_array_s[2][0] = -22.2; // GA
    m_array_s[2][1] = -27.2; // GC
    m_array_s[2][2] = -19.9; // GG
    m_array_s[2][3] = -22.4; // GT
    m_array_s[3][0] = -21.3; // TA
    m_array_s[3][1] = -22.2; // TC
    m_array_s[3][2] = -22.7; // TG
    m_array_s[3][3] = -22.2; // TT
}

/*!
 * Thermodynamics -- Destructor
 */
Thermodynamics::~Thermodynamics() {
    free(m_translation_table);
    free(m_buf);
}

/*!
 * Enable/Disable G+C check (in batch-mode!)
 *
 * \param min_gc Minimal allowed G+C value (0 <= value <= 100)
 * \param max_gc Minimal allowed G+C value (0 <= value <= 100)
 * \return true, if enabled (== values are valid).
 */
bool Thermodynamics::enable_gc_check(double min_gc, double max_gc) {
    if (min_gc <= max_gc && 0 <= min_gc && min_gc <= 100 && 0 <= max_gc
            && max_gc <= 100) {
        m_min_gc = min_gc;
        m_max_gc = max_gc;
        m_test_gc = true;
    }
    return m_test_gc;
}

void Thermodynamics::disable_gc_check() {
    m_test_gc = false;
    m_gc = 0;
}

/*!
 * Enable/Disable melting temperature check
 * (Relevant for batch-processing!)
 *
 * \param min_temp Min. allowed temperature (degrees celsius)
 * \param max_temp Max. allowed temperature (degrees celsius)
 * \return true, if enabled (== values are valid).
 */
bool Thermodynamics::enable_tm_check(double min_tm, double max_tm) {
    if (min_tm <= max_tm && -273 < min_tm && -273 < max_tm && min_tm < 273
            && max_tm < 273) {
        m_min_tm = min_tm;
        m_max_tm = max_tm;
        m_test_tm = true;
    }
    return m_test_tm;
}

void Thermodynamics::disable_tm_check() {
    m_test_tm = false;
    m_delta_h = 0;
    m_delta_s = 0;
    m_tm = 0;
}

/*!
 * Evaluation of a signature (in batch-mode!)
 *
 * \param signature Signature that should be evaluated.
 * \return false, is signature is filtered. Otherwise true.
 */
bool Thermodynamics::batch_process(const char *signature) {
    // Translate the signature into a computable form...
    translate(signature);

    // G+C content test...
    if (m_test_gc) {
        internal_basics();
        if (m_gc < m_min_gc || m_gc > m_max_gc)
            return false;
    }

    // Melting temperature test...
    if (m_test_tm) {
        internal_thermodynamics();
        if (m_tm < m_min_tm || m_tm > m_max_tm)
            return false;
    }

    // All tests were successful. Return true.
    return true;
}

/*!
 * Process a signature (without further evaluation)
 */
void Thermodynamics::process(const char *signature) {
    translate(signature);
    internal_basics();
    internal_thermodynamics();
}

/*!
 * Copy the signature into our buffer and translate it into
 * a manageable format.
 *
 * \param signature The signature string that should be evaluated.
 * \return Number of bases that could not be translated.
 * A successful run should return 0.
 */
unsigned int Thermodynamics::translate(const char *signature) {
    unsigned int errors = 0; // Error flag
    if (signature) {
        // Fetch length of the signature
        m_buf_size = strlen(signature);

        // Reallocate memory for our buffer, if necessary...
        if (m_buf_size > m_buf_vsize) {
            m_buf = (unsigned char *) realloc(m_buf,
                    m_buf_size * sizeof(unsigned char));
            if (!m_buf) {
                m_buf_size = 0;
                m_buf_vsize = 0;
                return false;
            }
        }
        // Create start pointers...
        const unsigned char *in = (const unsigned char*) signature;
        unsigned char *out = m_buf;
        while (*in) {
            *out = m_translation_table[(unsigned int) *in];
            ++in;
            if (*out == 0xFF)
                ++errors;
            else
                ++out;
        }
        m_buf_size -= errors;
    }
    return errors;
}

/*!
 * Internal G+C content calculation
 */
void Thermodynamics::internal_basics() {
    unsigned int basecount[4] = { 0, 0, 0, 0 };
    for (unsigned int i = 0; i < m_buf_size; ++i)
        ++basecount[(unsigned int) m_buf[i]];

    // Compute G+C content
    m_gc = (double) (basecount[1] + basecount[2]) * 100 / (double) m_buf_size;

    if (m_buf_size < 14) {
        // For lengths < 14 compute temperature according to:
        // Marmur,J., and Doty,P. (1962) J Mol Biol 5:109-118
        m_tm_basic = ((basecount[1] + basecount[2]) * 4
                + (basecount[0] + basecount[3]) * 2);
    } else {
        // For lengths > 13 compute temperature according to:
        // Wallace,R.B., Shaffer,J., Murphy,R.F., Bonner,J., Hirose,T., and
        // Itakura,K. (1979) Nucleic Acids Res 6:3543-3557
        m_tm_basic = 64.9
                + 41 * (basecount[1] + basecount[2] - 16.4) / m_buf_size;
    }
}

/*!
 * Internal melting temperature calculation.
 */
void Thermodynamics::internal_thermodynamics() {
    // Reset delta values
    m_delta_h = 0;
    m_delta_s = 0;

    // TODO: Predefined environment settings!
    // All concentrations in mmol/l (millimol/liter)
    double c_salt = 1000.0; // == 1 mol/l
    double c_mg = 0.0;
    double c_oligo = 0.00001; // == 10 nmol/l

    // Effect on entropy by salt correction. (Ahsen et al. 1999)
    // Increase of stability due to presence of Mg (--> effect on entropy).
    double salt_effect = (c_salt / 1000) + ((c_mg / 1000) * 140);
    m_delta_s += 0.368 * m_buf_size * log(salt_effect); // TODO: or (m_buf_size - 1)?

    // Primary/Terminal corrections. (SantaLucia1998)
    char firstnucleotide = m_buf[0];
    if (firstnucleotide == 0x01 || firstnucleotide == 0x02) {
        // Primary C or G
        m_delta_h += 0.1;
        m_delta_s += -2.8;
    } else {
        // Primary A or T
        m_delta_h += 2.3;
        m_delta_s += 4.1;
    }

    char lastnucleotide = m_buf[m_buf_size - 1];
    if (lastnucleotide == 0x01 || lastnucleotide == 0x02) {
        // Terminal C or G
        m_delta_h += 0.1;
        m_delta_s += -2.8;
    } else {
        // Terminal A or T
        m_delta_h += 2.3;
        m_delta_s += 4.1;
    }

    // Compute delta_h and delta_s with base stacking (SantaLucia1998)
    for (unsigned int i = 0; i < m_buf_size - 1; ++i) {
        unsigned int first = m_buf[i];
        unsigned int next = m_buf[i + 1];
        m_delta_h += m_array_h[first][next];
        m_delta_s += m_array_s[first][next];
    }

    // Compute the melting temperature.
    m_tm = ((1000 * m_delta_h) / (m_delta_s + (1.987 * log(c_oligo / 4000))))
                    - 273.15;
}

/*!
 * Get the G+C content value of the last processed signature.
 *
 * \return G+C value
 */
double Thermodynamics::get_gc_content() {
    return m_gc;
}

/*!
 * Get the melting temperature according to Marmur1962/Wallace1979.
 */
double Thermodynamics::get_tm_basic() {
    return m_tm_basic;
}

/*!
 * Get the temperature and thermodynamical parameters of
 * the last processed signature.
 */
double Thermodynamics::get_tm_base_stacking() {
    return m_tm;
}

double Thermodynamics::get_delta_g37() {
    return m_delta_h - (273.15 + 37) * m_delta_s / 1000;
}

double Thermodynamics::get_delta_g_temp(double temp) {
    if (-273 < temp && temp < 273)
        return m_delta_h - (273.15 + temp) * m_delta_s / 1000;
    return 0;
}

double Thermodynamics::get_delta_h() {
    return m_delta_h;
}

double Thermodynamics::get_delta_s() {
    return m_delta_s;
}
