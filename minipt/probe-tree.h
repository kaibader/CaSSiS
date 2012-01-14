/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: probe_tree.h
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef MINIPT_PROBE_TREE_H
#define MINIPT_PROBE_TREE_H

#include <cassert>
#include <cstdio>

#ifdef _WIN32
#else
#include <bits/wordsize.h>
#endif

#ifndef MINIPT_PROBE_H
#include "probe.h"
#endif

namespace minipt {

#define PTM_magic             0xf4
#define PTM_TABLE_SIZE        (1024*256)
#define PTM_ALIGNED           1
#define PTM_LD_ALIGNED        0
#define PTM_MAX_TABLES        256 // -- ralf testing
#define PTM_MAX_SIZE          (PTM_MAX_TABLES*PTM_ALIGNED)
#define PT_CHAIN_END          0xff
#define PT_CHAIN_NTERM        250
#define PT_SHORT_SIZE         0xffff
#define PT_BLOCK_SIZE         0x800
#define PT_INIT_CHAIN_SIZE    20

typedef void * PT_PNTR;

extern struct PTM_struct {
    char *data;
    int size;
    long allsize;
    char *tables[PTM_MAX_TABLES + 1];
#ifdef DEBUG
    long debug[PTM_MAX_TABLES+1];
#endif
    PT_NODE_TYPE flag_2_type[256];
    //
    void **alloc_ptr;
    unsigned long alloc_counter;
    unsigned long alloc_array_size;
} PTM;

extern char PT_count_bits[PT_B_MAX + 1][256]; // returns how many bits are set
// e.g. PT_count_bits[3][n] is the number of the 3 lsb bits

#define IS_SINGLE_BRANCH_NODE 0x40
#ifdef ARB_64
# define INT_SONS              0x80
# define LONG_SONS             0x40
#else
# define LONG_SONS             0x80
#endif

// -----------------------------------------------
//      Get the size of entries (stage 1) only

#define PT_EMPTY_LEAF_SIZE       (1+sizeof(PT_PNTR)+6) // tag father name rel apos
#define PT_LEAF_SIZE(leaf)       (1+sizeof(PT_PNTR)+6+2*PT_count_bits[3][leaf->flags])
#define PT_EMPTY_CHAIN_SIZE      (1+sizeof(PT_PNTR)+2+sizeof(PT_PNTR)) // tag father apos first_elem
#define PT_EMPTY_NODE_SIZE       (1+sizeof(PT_PNTR)) // tag father
#define PT_NODE_COUNT_SONS(leaf) PT_count_bits[3][leaf->flags];
#define PT_NODE_SIZE(node, size) size = PT_EMPTY_NODE_SIZE + sizeof(PT_PNTR)*PT_count_bits[PT_B_MAX][node->flags]

// ----------------------------
//      Read and write type

#define PT_GET_TYPE(pt)     (PTM.flag_2_type[pt->flags])
#define PT_SET_TYPE(pt, i, j) (pt->flags = (i<<6)+j)

// ----------------------
//      bswap for OSX

#ifdef WIN32
static inline unsigned short bswap_16(unsigned short x) {
    return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
    return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
    return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}
#else
#include <byteswap.h>
#endif // DARWIN
// ------------------------------------------------------------
// Note about bswap as used here:
//
// * MSB has to be at start of written byte-chain, cause the most significant bit is used to separate
//   between INT and SHORT
//
// * To use PT-server on a big-endian system it has to be skipped

// ---------------------------------
//      Read and write to memory

#define PT_READ_INT(ptr, my_int_i)                                      \
        do {                                                                \
            unsigned int *uiptr = (unsigned int*)(ptr);                     \
            (my_int_i)=(unsigned int)bswap_32(*uiptr);                      \
        } while (0)

#define PT_WRITE_INT(ptr, my_int_i)                                     \
        do {                                                                \
            unsigned int *uiptr = (unsigned int*)(ptr);                     \
            *uiptr              = bswap_32((unsigned int)(my_int_i));       \
        } while (0)

#define PT_READ_SHORT(ptr, my_int_i)                    \
        do {                                                \
            (my_int_i) = bswap_16(*(unsigned short*)(ptr)); \
        } while (0)

#define PT_WRITE_SHORT(ptr, my_int_i)                                   \
        do {                                                                \
            unsigned short *usptr = (unsigned short*)(ptr);                 \
            *usptr                = bswap_16((unsigned short)(my_int_i));   \
        } while (0)

#define PT_WRITE_CHAR(ptr, my_int_i) do { *(unsigned char *)(ptr) = my_int_i; } while (0)

#define PT_READ_CHAR(ptr, my_int_i) do { my_int_i = *(unsigned char *)(ptr); } while (0)

#ifdef ARB_64

# define PT_READ_PNTR(ptr, my_int_i)                            \
        do {                                                        \
            assert(sizeof(my_int_i) == 8);                       \
            unsigned long *ulptr = (unsigned long*)(ptr);           \
            (my_int_i)           = (unsigned long)bswap_64(*ulptr); \
        } while (0)

# define PT_WRITE_PNTR(ptr, my_int_i)                                   \
        do {                                                                \
            unsigned long *ulptr = (unsigned long*)(ptr);                   \
            *ulptr               = bswap_64((unsigned long)(my_int_i));     \
        } while (0)

#else
// not ARB_64

# define PT_READ_PNTR(ptr, my_int_i) PT_READ_INT(ptr, my_int_i)
# define PT_WRITE_PNTR(ptr, my_int_i) PT_WRITE_INT(ptr, my_int_i)

#endif

#define PT_WRITE_NAT(ptr, i)                    \
        do {                                        \
            assert(i >= 0);                      \
            if (i >= 0x7FFE)                        \
            {                                       \
                PT_WRITE_INT(ptr, i|0x80000000);    \
                ptr += sizeof(int);                 \
            }                                       \
            else                                    \
            {                                       \
                PT_WRITE_SHORT(ptr, i);             \
                ptr += sizeof(short);               \
            }                                       \
        } while (0)

#define PT_READ_NAT(ptr, i)                                             \
        do {                                                                \
            if (*ptr & 0x80) {                                              \
                PT_READ_INT(ptr, i); ptr += sizeof(int); i &= 0x7fffffff;   \
            }                                                               \
            else {                                                          \
                PT_READ_SHORT(ptr, i); ptr += sizeof(short);                \
            }                                                               \
        } while (0)

inline const char *PT_READ_CHAIN_ENTRY(const char* ptr, int mainapos, int *name,
        int *apos, int *rpos) {
    // Caution: 'name' has to be initialized before first call and shall not be modified between calls

    *apos = 0;
    *rpos = 0;

    unsigned char *rcep = (unsigned char*) ptr;
    unsigned int rcei = (*rcep);

    if (rcei == PT_CHAIN_END) {
        *name = -1;
        ptr++;
    } else {
        if (rcei & 0x80) {
            if (rcei & 0x40) {
                PT_READ_INT(rcep, rcei);
                rcep += 4;
                rcei &= 0x3fffffff;
            } else {
                PT_READ_SHORT(rcep, rcei);
                rcep += 2;
                rcei &= 0x3fff;
            }
        } else {
            rcei &= 0x7f;
            rcep++;
        }
        *name += rcei;
        rcei = (*rcep);

        bool isapos = rcei & 0x80;

        if (rcei & 0x40) {
            PT_READ_INT(rcep, rcei);
            rcep += 4;
            rcei &= 0x3fffffff;
        } else {
            PT_READ_SHORT(rcep, rcei);
            rcep += 2;
            rcei &= 0x3fff;
        }
        *rpos = (int) rcei;
        if (isapos) {
            rcei = (*rcep);
            if (rcei & 0x80) {
                PT_READ_INT(rcep, rcei);
                rcep += 4;
                rcei &= 0x7fffffff;
            } else {
                PT_READ_SHORT(rcep, rcei);
                rcep += 2;
                rcei &= 0x7fff;
            }
            *apos = (int) rcei;
        } else {
            *apos = (int) mainapos;
        }
        ptr = (char *) rcep;
    }

    return ptr;
}

inline char *PT_WRITE_CHAIN_ENTRY(const char * const ptr, const int mainapos,
        int name, const int apos, const int rpos) { // stage 1
    unsigned char *wcep = (unsigned char *) ptr;
    int isapos;
    if (name < 0x7f) { // write the name
        *(wcep++) = name;
    } else if (name < 0x3fff) {
        name |= 0x8000;
        PT_WRITE_SHORT(wcep, name);
        wcep += 2;
    } else {
        name |= 0xc0000000;
        PT_WRITE_INT(wcep, name);
        wcep += 4;
    }

    if (apos == mainapos)
        isapos = 0;
    else
        isapos = 0x80;

    if (rpos < 0x3fff) { // write the rpos
        // 0x7fff, mit der rpos vorher verglichen wurde war zu gross
        PT_WRITE_SHORT(wcep, rpos);
        *wcep |= isapos;
        wcep += 2;
    } else {
        PT_WRITE_INT(wcep, rpos);
        *wcep |= 0x40 + isapos;
        wcep += 4;
    }
    if (isapos) { // write the apos
        if (apos < 0x7fff) {
            PT_WRITE_SHORT(wcep, apos);
            wcep += 2;
        } else {
            PT_WRITE_INT(wcep, apos);
            *wcep |= 0x80;
            wcep += 4;
        }
    }
    return (char *) wcep;
}
// calculate the index of the pointer in a node

inline POS_TREE *PT_read_son_stage_3(PTM2 *ptmain, POS_TREE *node,
        PT_BASES base) {
    long i;
    unsigned int sec;
    unsigned int offset;

    // Only allow calls during stage_3!
    assert(ptmain->stage3);

    if (node->flags & IS_SINGLE_BRANCH_NODE) {
        if (base != (node->flags & 0x7))
            return NULL; // no son
        i = (node->flags >> 3) & 0x7; // this son
        if (!i)
            i = 1;
        else
            i += 2; // offset mapping
        assert(i >= 0);
        return (POS_TREE *) (((char *) node) - i);
    }
    if (!((1 << base) & node->flags))
        return NULL; // bit not set
    sec = (unsigned char) node->data; // read second byte for charshort/shortlong info
    i = PT_count_bits[base][node->flags];
    i += PT_count_bits[base][sec];
#ifdef ARB_64
    if (sec & LONG_SONS) {
        if (sec & INT_SONS) { // undefined -> error
            printf("Your pt-server search tree is corrupt! You can not use it anymore.\n"
                    "Error: ((sec & LONG_SON) && (sec & INT_SONS)) == true\n"
                    "       this combination of both flags is not implemented\n");
            exit(EXIT_FAILURE);
        }
        else { // long/int
#if 0
            // Warning disabled!
            printf("Warning: A search tree of this size is not tested.\n");
            printf("         (sec & LONG_SON) == true\n");
#endif
            offset = 4 * i;
            if ((1<<base) & sec) { // long
                PT_READ_PNTR((&node->data+1)+offset, i);
            }
            else { // int
                PT_READ_INT((&node->data+1)+offset, i);
            }
        }

    }
    else {
        if (sec & INT_SONS) { // int/short
            offset = i+i;
            if ((1<<base) & sec) { // int
                PT_READ_INT((&node->data+1)+offset, i);
            }
            else { // short
                PT_READ_SHORT((&node->data+1)+offset, i);
            }
        }
        else { // short/char
            offset = i;
            if ((1<<base) & sec) { // short
                PT_READ_SHORT((&node->data+1)+offset, i);
            }
            else { // char
                PT_READ_CHAR((&node->data+1)+offset, i);
            }
        }
    }
#else
    if (sec & LONG_SONS) {
        offset = i + i;
        if ((1 << base) & sec) {
            PT_READ_INT((&node->data+1)+offset, i);
        } else {
            PT_READ_SHORT((&node->data+1)+offset, i);
        }
    } else {
        offset = i;
        if ((1 << base) & sec) {
            PT_READ_SHORT((&node->data+1)+offset, i);
        } else {
            PT_READ_CHAR((&node->data+1)+offset, i);
        }
    }
#endif
    assert(i >= 0);
    return (POS_TREE *) (((char*) node) - i);
}

inline POS_TREE *PT_read_son_stage_1(PTM2 *ptmain, POS_TREE *node,
        PT_BASES base) {
    // Only allow calls during stage_1!
    assert(!ptmain->stage3);

    long i;
    if (!((1 << base) & node->flags))
        return NULL; // bit not set
    base = (PT_BASES) PT_count_bits[base][node->flags];
    PT_READ_PNTR((&node->data)+sizeof(PT_PNTR)*base+ptmain->mode, i);
    return (POS_TREE *) (i + ptmain->data_start); // ptmain->data_start == 0x00 in stage 1
}

inline PT_NODE_TYPE PT_read_type(POS_TREE *node) {
    return (PT_NODE_TYPE) PT_GET_TYPE(node);
}

inline int PT_read_name(PTM2 *ptmain, POS_TREE *node) {
    int i;
    if (node->flags & 1) {
        PT_READ_INT((&node->data)+ptmain->mode, i);
    } else {
        PT_READ_SHORT((&node->data)+ptmain->mode, i);
    }
    assert(i >= 0);
    return i;
}

inline int PT_read_rpos(PTM2 *ptmain, POS_TREE *node) {
    int i;
    char *data = (&node->data) + 2 + ptmain->mode;
    if (node->flags & 1)
        data += 2;
    if (node->flags & 2) {
        PT_READ_INT(data, i);
    } else {
        PT_READ_SHORT(data, i);
    }
    assert(i >= 0);
    return i;
}

inline int PT_read_apos(PTM2 *ptmain, POS_TREE *node) {
    int i;
    char *data = (&node->data) + ptmain->mode + 4; // father 4 name 2 rpos 2
    if (node->flags & 1)
        data += 2;
    if (node->flags & 2)
        data += 2;
    if (node->flags & 4) {
        PT_READ_INT(data, i);
    } else {
        PT_READ_SHORT(data, i);
    }
    assert(i >= 0);
    return i;
}

struct DataLoc {
    int name;
    int apos;
    int rpos;

    void init(const char ** data, int pos) {
        *data = PT_READ_CHAIN_ENTRY(*data, pos, &name, &apos, &rpos);
    }
    void init(PTM2 *ptmain, POS_TREE *pt) {
        assert(PT_read_type(pt) == PT_NT_LEAF);

        name = PT_read_name(ptmain, pt);
        apos = PT_read_apos(ptmain, pt);
        rpos = PT_read_rpos(ptmain, pt);
    }

    DataLoc(int name_, int apos_, int rpos_) :
        name(name_), apos(apos_), rpos(rpos_) {
    }
    DataLoc(const char ** data, int pos) :
        name(0), apos(0), rpos(0) {
        init(data, pos);
    }
    DataLoc(PTM2 *ptmain, POS_TREE *pt) :
        name(0), apos(0), rpos(0) {
        init(ptmain, pt);
    }

#if defined(DEBUG)
    void dump(FILE *fp) const {
        fprintf(fp, "          apos=%6i  rpos=%6i  name=%6i\n", apos, rpos, name);
        fflush(fp);
    }
#endif
};

template<typename T>
int PT_forwhole_chain(PTM2 *ptmain, POS_TREE *node, T func) {
    unsigned int type = PT_read_type(node);
    assert(type == PT_NT_CHAIN);
    // assert(PT_read_type(node) == PT_NT_CHAIN);

    const char *data = (&node->data) + ptmain->mode;
    int pos;

    if (node->flags & 1) {
        PT_READ_INT(data, pos);
        data += 4;
    } else {
        PT_READ_SHORT(data, pos);
        data += 2;
    }

    int error = 0;
    DataLoc location(&data, pos);
    while (location.name >= 0) {
        error = func(location);
        if (error)
            break;
        location.init(&data, pos);
    }
    return error;
}

template<typename T>
int PT_withall_tips(PTM2 *ptmain, POS_TREE *node, T func) {
    // like PT_forwhole_chain, but also can handle leafs
    PT_NODE_TYPE type = PT_read_type(node);
    if (type == PT_NT_LEAF) {
        return func(DataLoc(ptmain, node));
    }

    assert(type == PT_NT_CHAIN);
    return PT_forwhole_chain(ptmain, node, func);
}

#if defined(DEBUG)
struct PTD_chain_print {int operator()(const DataLoc& loc) {loc.dump(stdout); return 0;}};
#endif

} /* namespace minipt */

#endif /* MINIPT_PROBE_TREE_H */
