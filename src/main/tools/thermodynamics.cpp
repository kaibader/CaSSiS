/*!
 * Thermodynamic Parameter Test Tool
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <cassis/thermodynamics.h>

/*!
 * Generate a random sequence...
 */
void gen_sequence(char *buf, unsigned int min_len, unsigned int max_len) {
    unsigned int i = 0;
    for (; i < (rand() % (max_len - min_len) + min_len); i++) {
        int j = rand() % 4;
        switch (j) {
        case 0:
            buf[i] = 'A';
            break;
        case 1:
            buf[i] = 'C';
            break;
        case 2:
            buf[i] = 'G';
            break;
        case 3:
        default:
            buf[i] = 'T';
            break;
        }
    }
    buf[i] = 0x00;
}

int main(int argc, char **argv) {
    // Our thermodynamics class
    Thermodynamics thermo;

    // Allow a parameter, a signature string that should be evaluated.
    // Otherwise run a stress test on the batch process...
    if (argc == 2) {
        thermo.process(argv[1]);

        printf("\tG+C-content= %f * 100 percent\n\t"
                "Melting temp. according to Marmur1962/Wallace1979: %f\n\n",
                thermo.get_gc_content(), thermo.get_tm_basic());

        printf("\tTm= %f\n\tdeltaG_37= %f\n\tdeltaG_46= %f\n\t"
                "enthalpy(deltaH)= %f\n\tentropy(deltaS)= %f\n",
                thermo.get_tm_base_stacking(), thermo.get_delta_g37(),
                thermo.get_delta_g_temp(46), thermo.get_delta_h(),
                thermo.get_delta_s());

        return EXIT_SUCCESS;
    }

    // DEFINITIONS...
    unsigned int repeats = 100000;
    char *buf = (char *) malloc(32 * sizeof(char));

    // Repeat computations...
    for (unsigned int i = 0; i < repeats; ++i) {
        gen_sequence(buf, 15, 20);

        // Evaluation...
        thermo.batch_process(buf);
    }

    free(buf);
    return EXIT_SUCCESS;
}
