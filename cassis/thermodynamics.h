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

#ifndef THERMODYNAMICS_H_
#define THERMODYNAMICS_H_

/*!
 * Thermodynamics class
 */
class Thermodynamics {
public:
    /*!
     * Thermodynamics -- Constructor & Destructor
     */
    Thermodynamics();
    virtual ~Thermodynamics();

    /*!
     * Enable/Disable G+C check
     * (when using batch_process)
     *
     * \param min_gc Min. allowed G+C percentage (0 <= value <= 1)
     * \param max_gc Max. allowed G+C percentage (0 <= value <= 1)
     * \return true, if enabled (== values are valid).
     */
    bool enable_gc_check(double min_gc, double max_gc);
    void disable_gc_check();

    /*!
     * Enable/Disable melting temperature check
     * (when using batch_process)
     *
     * \param min_temp Min. allowed temperature (degrees celsius)
     * \param max_temp Max. allowed temperature (degrees celsius)
     * \return true, if enabled (== values are valid).
     */
    bool enable_tm_check(double min_tm, double max_tm);
    void disable_tm_check();

    /*!
     * Batch-process a signature
     *
     * \param signature Signature that should be evaluated.
     * \return True, if signature matches all criteria, otherwise false.
     */
    bool batch_process(const char *signature);

    /*!
     * Process a signature (without further evaluation)
     *
     * \param signature Signature that should be evaluated.
     */
    void process(const char *signature);

    /*!
     * Get the G+C content value of the last processed signature.
     *
     * \return G+C value (in percent)
     */
    double get_gc_content();

    /*!
     * Get the melting temperature according to Marmur1962/Wallace1979
     * of the last processed signature.
     */
    double get_tm_basic();

    /*!
     * Get the temperature and thermodynamical parameters of
     * the last processed signature.
     */
    double get_tm_base_stacking();
    double get_delta_g37();
    double get_delta_g_temp(double temp);
    double get_delta_h();
    double get_delta_s();
private:
    /*!
     * Copy constructor.
     * Not implemented --> private.
     */
    Thermodynamics(const Thermodynamics&);

    /*!
     * Assignment operator.
     * Not implemented. --> private.
     */
    Thermodynamics &operator=(const Thermodynamics&);

    /*!
     * Copy the signature into our buffer and translate it into
     * a manageable format.
     *
     * \param signature The signature string that should be evaluated.
     * \return false, if an error occurred during translation. Otherwise true.
     */
    unsigned int translate(const char *signature);

    /*!
     * Flags, used to enable/disable certain tests...
     */
    bool m_test_gc;
    bool m_test_tm;
    bool m_test_tm_basic;

    /*!
     * Internal. Basic calculations:
     *   - G+C content
     *   - Molar weight
     *   - Basic melting temperature according to Marmur1962/Wallace1979
     */
    void internal_basics();
    //
    double m_min_gc;
    double m_max_gc;
    //
    double m_gc;
    double m_tm_basic;

    /*!
     * Internal. Various extended thermodynamical calculations.
     */
    void internal_thermodynamics();
    //
    double m_min_tm;
    double m_max_tm;
    //
    double m_delta_h;
    double m_delta_s;
    double m_tm;

    /*!
     * Base translation, entropy and enthalpy tables...
     */
    unsigned char *m_translation_table;
    double m_array_h[4][4];
    double m_array_s[4][4];

    /*!
     * (Translated) signature buffer...
     */
    unsigned int m_buf_size;
    unsigned int m_buf_vsize;
    unsigned char *m_buf;
};

#endif /* THERMODYNAMICS_H_ */
