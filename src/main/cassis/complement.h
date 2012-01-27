/*!
 * Translates an oligonucleotide into its complement.
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

#ifndef COMPLEMENT_H_
#define COMPLEMENT_H_

#include <cstring>

/*!
 * Translates a nucleotide into its complement.
 * \param c Nucleotide character that should be translated.
 * \param RNA Should we translate into 'T' (=DNA, false) or 'U' (=RNA, true).
 * \return Translated character
 */
static inline char complementNucleotide(char c, bool RNA = false) {
    char n = c;

    switch (c) {
    case 'A':
        n = RNA ? 'U' : 'T';
        break;
    case 'a':
        n = RNA ? 'u' : 't';
        break;
    case 'U':
    case 'T':
        n = 'A';
        break;
    case 'u':
    case 't':
        n = 'a';
        break;

    case 'C':
        n = 'G';
        break;
    case 'c':
        n = 'g';
        break;
    case 'G':
        n = 'C';
        break;
    case 'g':
        n = 'c';
        break;

    case 'M':
        n = 'K';
        break;
    case 'm':
        n = 'k';
        break;
    case 'K':
        n = 'M';
        break;
    case 'k':
        n = 'm';
        break;

    case 'R':
        n = 'Y';
        break;
    case 'r':
        n = 'y';
        break;
    case 'Y':
        n = 'R';
        break;
    case 'y':
        n = 'r';
        break;

    case 'V':
        n = 'B';
        break;
    case 'v':
        n = 'b';
        break;
    case 'B':
        n = 'V';
        break;
    case 'b':
        n = 'v';
        break;

    case 'H':
        n = 'D';
        break;
    case 'h':
        n = 'd';
        break;
    case 'D':
        n = 'H';
        break;
    case 'd':
        n = 'h';
        break;

    case 'S':
    case 's':
    case 'W':
    case 'w':
    case 'N':
    case 'n':
    case '.':
    case '-':
        break;

    default:
        break;
    }

    return n;
}

/*!
 * Translates a nucleotide into its complement.
 * \param seq Sequence that should be translated.
 * \param RNA Should we translate into 'T' (=DNA, false) or 'U' (=RNA, true).
 * \return Translated sequence (== reverse complement) allocated with malloc.
 */
char *reverseComplementSequence(const char *seq, bool RNA = false) {
    size_t len = strlen(seq);
    char *rc_seq = (char *) malloc(+1);
    for (size_t i = 0; i < len; ++i)
        rc_seq[i] = complementNucleotide(seq[len - i], RNA);
    rc_seq[len] = 0;
    return rc_seq;
}

#endif /* COMPLEMENT_H_ */
