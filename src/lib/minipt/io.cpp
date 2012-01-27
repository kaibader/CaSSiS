/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: PT_io.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "probe.h"
#include "io.h"

#include <cstdlib>
#include <cassert>
#include <stdint.h>

namespace minipt {

int compress_data(char *probestring) {
    /*! change a sequence with normal bases the PT_? format and delete all other signs */
    char c;
    char *src, *dest;
    dest = src = probestring;

    while ((c = *(src++))) {
        switch (c) {
        case 'A':
        case 'a':
            *(dest++) = PT_A;
            break;
        case 'C':
        case 'c':
            *(dest++) = PT_C;
            break;
        case 'G':
        case 'g':
            *(dest++) = PT_G;
            break;
        case 'U':
        case 'u':
        case 'T':
        case 't':
            *(dest++) = PT_T;
            break;
        case 'N':
        case 'n':
            *(dest++) = PT_N;
            break;
        default:
            break;
        }

    }
    *dest = PT_QU;
    return 0;
}

inline size_t count_uint_32(uint32_t *seq, size_t seqsize, uint32_t cmp) {
    size_t count = 0;
    while (count < seqsize && seq[count] == cmp)
        count++;
    return count * 4;
}

inline size_t count_char(const char *seq, size_t seqsize, char c, uint32_t c4) {
    if (seq[0] == c) {
        size_t count = 1
                + count_uint_32((uint32_t*) (seq + 1), (seqsize - 1) / 4, c4);
        for (; count < seqsize && seq[count] == c; ++count)
            ;
        return count;
    }
    return 0;
}

inline size_t count_dots(const char *seq, int seqsize) {
    return count_char(seq, seqsize, '.', 0x2E2E2E2E);
}
inline size_t count_gaps(const char *seq, int seqsize) {
    return count_char(seq, seqsize, '-', 0x2D2D2D2D);
}

inline size_t count_gaps_and_dots(const char *seq, int seqsize) {
    size_t count = 0;
    size_t count2;
    size_t count3;

    do {
        count2 = count_dots(seq + count, seqsize - count);
        count += count2;
        count3 = count_gaps(seq + count, seqsize - count);
        count += count3;
    } while (count2 || count3);
    return count;
}

// helper to use char as array index:
inline unsigned char safeCharIndex(char c) {
    return static_cast<unsigned char>(c);
}

static unsigned char *compress_table = NULL;

void cleanup_probe_compress_sequence() {
    free(compress_table);
}

int probe_compress_sequence(char *seq, int seqsize) {
    if (compress_table == NULL) {
        compress_table = (unsigned char *) malloc(256 * sizeof(char));
        memset(compress_table, PT_N, 256);
        compress_table['A'] = compress_table['a'] = PT_A;
        compress_table['C'] = compress_table['c'] = PT_C;
        compress_table['G'] = compress_table['g'] = PT_G;
        compress_table['T'] = compress_table['t'] = PT_T;
        compress_table['U'] = compress_table['u'] = PT_T;
        compress_table['.'] = PT_QU;
        compress_table[0] = PT_B_UNDEF;
    }

    char *dest = seq;
    size_t offset = 0;

    while (seq[offset]) {
        offset += count_gaps(seq + offset, seqsize - offset); // skip over gaps

        unsigned char c = compress_table[safeCharIndex(seq[offset++])];
        if (c == PT_B_UNDEF)
            break; // already seen terminal zerobyte

        *dest++ = c;
        if (c == PT_QU) { // TODO: *seq = '.' ???
            // Skip over gaps and dots
            offset += count_gaps_and_dots(seq + offset, seqsize - offset);
            // dest[-1] = PT_N; // TODO: Handle '.' like 'N'. Experimental!
        }
    }

    if (dest[-1] != PT_QU) {
        *dest++ = PT_QU;
    }

#ifdef ARB_64
    assert(!((dest - seq) & 0xffffffff00000000)); // must fit into 32 bit
#endif

    return dest - seq;
}

} /* namespace minipt */
