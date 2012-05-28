/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: PT_prefixtree.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "probe.h"
#include "probe-tree.h"
#include "prefixtree.h"

#include <sys/types.h>
#ifdef _WIN32
#else
#include <sys/mman.h>
#endif
#include <climits>
#include <sys/stat.h>

namespace minipt {

struct PTM_struct PTM;
char PT_count_bits[PT_B_MAX + 1][256]; // returns how many bits are set

void PT_init_count_bits() {
    unsigned int base;
    unsigned int count;
    unsigned int i, h, j;
    for (base = PT_QU; base <= PT_B_MAX; base++) {
        for (i = 0; i < 256; i++) {
            h = 0xff >> (8 - base);
            h &= i;
            count = 0;
            for (j = 0; j < 8; j++) {
                if (h & 1)
                    count++;
                h = h >> 1;
            }
            PT_count_bits[base][i] = count;
        }
    }
}

PTM2 *PT_init() {
    PTM2 *ptmain;
    memset(&PTM, 0, sizeof(struct PTM_struct));
    ptmain = (PTM2 *) calloc(1, sizeof(PTM2));
    ptmain->mode = sizeof(PT_PNTR);
    ptmain->stage1 = 1;
    int i;
    for (i = 0; i < 256; i++) {
        if ((i & 0xe0) == 0x20)
            PTM.flag_2_type[i] = PT_NT_SAVED;
        else if ((i & 0xe0) == 0x00)
            PTM.flag_2_type[i] = PT_NT_LEAF;
        else if ((i & 0x80) == 0x80)
            PTM.flag_2_type[i] = PT_NT_NODE;
        else if ((i & 0xe0) == 0x40)
            PTM.flag_2_type[i] = PT_NT_CHAIN;
        else
            PTM.flag_2_type[i] = PT_NT_UNDEF;
    }
    PT_init_count_bits();

    PTM.alloc_ptr = NULL;
    PTM.alloc_counter = 0;
    PTM.alloc_array_size = 0;

    return ptmain;
}

// ------------------------------
//      functions for stage 1

void PT_change_father(POS_TREE *father, POS_TREE *source, POS_TREE *dest) { // stage 1
    long i, j;
    i = PT_count_bits[PT_B_MAX][father->flags];
    for (; i > 0; i--) {
        PT_READ_PNTR((&father->data)+sizeof(PT_PNTR)*i, j);
        if (j == (long) source) {
            PT_WRITE_PNTR((&father->data)+sizeof(PT_PNTR)*i, (long)dest);
            return;
        }
    }
    abort();
}

POS_TREE *PT_add_to_chain(PTM2 *ptmain, POS_TREE *node, int name, int apos,
        int rpos) // stage1
{ // insert at the beginning of list
    static char buffer[100];
    unsigned long old_first;
    char *data;
    // int mainapos = 0; // TODO: UNUSED?
    data = (&node->data) + ptmain->mode;
    if (node->flags & 1) {
        // PT_READ_INT(data, mainapos); // TODO: UNUSED?
        data += 4;
    } else {
        // PT_READ_SHORT(data, mainapos); // TODO: UNUSED?
        data += 2;
    }PT_READ_PNTR(data, old_first);
    // create a new list element
    char *p;
    p = buffer;
    PT_WRITE_PNTR(p, old_first);
    p += sizeof(PT_PNTR);
    PT_WRITE_NAT(p, name);
    PT_WRITE_NAT(p, rpos);
    PT_WRITE_NAT(p, apos);
    int size = p - buffer;
    p = (char *) calloc(1, size);
    memcpy(p, buffer, size);
    PT_WRITE_PNTR(data, p);
    ptstruct.stat.cut_offs++;
    return NULL;
}

POS_TREE *PT_change_leaf_to_node(PTM2 * /* ptmain */, POS_TREE *node) // stage 1
{
    long i;
    POS_TREE *father, *new_elem;
    if (PT_GET_TYPE(node) != PT_NT_LEAF)
        abort();
    PT_READ_PNTR((&node->data), i);
    father = (POS_TREE *) i;
    new_elem = (POS_TREE *) calloc(1, PT_EMPTY_NODE_SIZE);
    if (father)
        PT_change_father(father, node, new_elem);
    free(node);
    PT_SET_TYPE(new_elem, PT_NT_NODE, 0);
    PT_WRITE_PNTR((&(new_elem->data)), (long)father);
    return new_elem;
}

POS_TREE *PT_leaf_to_chain(PTM2 *ptmain, POS_TREE *node) // stage 1
{
    long i;
    int apos, rpos, name;
    POS_TREE *father, *new_elem;
    int chain_size;
    char *data;
    if (PT_GET_TYPE(node) != PT_NT_LEAF)
        abort();
    PT_READ_PNTR((&node->data), i);
    father = (POS_TREE *) i;
    name = PT_read_name(ptmain, node); // backup name,
    apos = PT_read_apos(ptmain, node); //        apos,
    rpos = PT_read_rpos(ptmain, node); //        rpos
    chain_size = PT_EMPTY_CHAIN_SIZE;
    if (apos > PT_SHORT_SIZE)
        chain_size += 2;

    new_elem = (POS_TREE *) calloc(1, chain_size);
    PT_change_father(father, node, new_elem);
    free(node);
    PT_SET_TYPE(new_elem, PT_NT_CHAIN, 0);
    PT_WRITE_PNTR((&new_elem->data), (long)father);
    // father
    data = (&new_elem->data) + sizeof(PT_PNTR);
    if (apos > PT_SHORT_SIZE) { // mainapos
        PT_WRITE_INT(data, apos);
        // .
        data += 4;
        new_elem->flags |= 1; // .
    } else { // .
        PT_WRITE_SHORT(data, apos);
        // .
        data += 2; // .
    }PT_WRITE_PNTR(data, NULL);
    // first element
    PT_add_to_chain(ptmain, new_elem, name, apos, rpos);
    return new_elem;
}

POS_TREE *PT_create_leaf(PTM2 *ptmain, POS_TREE ** pfather, PT_BASES base,
        int rpos, int apos, int name) {
    POS_TREE *father, *node, *new_elemfather;
    int base2;
    int leafsize;
    char *dest;
    leafsize = PT_EMPTY_LEAF_SIZE;
    if (rpos > PT_SHORT_SIZE)
        leafsize += 2;
    if (apos > PT_SHORT_SIZE)
        leafsize += 2;
    if (name > PT_SHORT_SIZE)
        leafsize += 2;
    node = (POS_TREE *) calloc(1, leafsize);
    if (base >= PT_B_MAX)
        assert(0);
    if (pfather) {
        int oldfathersize;
        POS_TREE *gfather, *son;
        long i, j, sbase, dbase;
        father = *pfather;
        PT_NODE_SIZE(father, oldfathersize);
        new_elemfather = (POS_TREE *) calloc(1,
                oldfathersize + sizeof(PT_PNTR));
        PT_READ_PNTR(&father->data, j);
        gfather = (POS_TREE *) j;
        if (gfather) {
            PT_change_father(gfather, father, new_elemfather);
            PT_WRITE_PNTR(&new_elemfather->data, gfather);
        }
        sbase = dbase = sizeof(PT_PNTR);
        base2 = 1 << base;
        for (i = 1; i < 0x40; i = i << 1) {
            if (i & father->flags) {
                PT_READ_PNTR(((char *) &father->data) + sbase, j);
                son = (POS_TREE *) j;
                int sontype = PT_GET_TYPE(son);
                if (sontype != PT_NT_SAVED) {
                    PT_WRITE_PNTR((char *) &son->data, new_elemfather);
                }PT_WRITE_PNTR(((char *) &new_elemfather->data) + dbase, son);
                sbase += sizeof(PT_PNTR);
                dbase += sizeof(PT_PNTR);
                continue;
            }
            if (i & base2) {
                PT_WRITE_PNTR(((char *) &new_elemfather->data) + dbase, node);
                PT_WRITE_PNTR((&node->data), (PT_PNTR)new_elemfather);
                dbase += sizeof(PT_PNTR);
            }
        }
        new_elemfather->flags = father->flags | base2;
        free(father);
        PT_SET_TYPE(node, PT_NT_LEAF, 0);
        *pfather = new_elemfather;
    }PT_SET_TYPE(node, PT_NT_LEAF, 0);
    dest = (&node->data) + sizeof(PT_PNTR);
    if (name > PT_SHORT_SIZE) {
        PT_WRITE_INT(dest, name);
        node->flags |= 1;
        dest += 4;
    } else {
        PT_WRITE_SHORT(dest, name);
        dest += 2;
    }
    if (rpos > PT_SHORT_SIZE) {
        PT_WRITE_INT(dest, rpos);
        node->flags |= 2;
        dest += 4;
    } else {
        PT_WRITE_SHORT(dest, rpos);
        dest += 2;
    }
    if (apos > PT_SHORT_SIZE) {
        PT_WRITE_INT(dest, apos);
        node->flags |= 4;
        dest += 4;
    } else {
        PT_WRITE_SHORT(dest, apos);
        dest += 2;
    }
    if (base == PT_QU)
        return PT_leaf_to_chain(ptmain, node);
    return node;
}

// ------------------------------------
//      functions for stage 1: save

void PTD_clear_fathers(PTM2 *ptmain, POS_TREE * node) // stage 1
{
    POS_TREE *sons;
    int i;
    PT_NODE_TYPE type = PT_read_type(node);
    if (type == PT_NT_SAVED)
        return;PT_WRITE_PNTR((&node->data), NULL);
    if (type == PT_NT_NODE) {
        for (i = PT_QU; i < PT_B_MAX; i++) {
            sons = PT_read_son_stage_1(ptmain, node, (enum PT_bases_enum) i);
            if (sons)
                PTD_clear_fathers(ptmain, sons);
        }
    }
}

#ifdef ARB_64
void PTD_put_longlong(FILE * out, unsigned long i)
{
    assert(i == (unsigned long) i);
    int io;
    static unsigned char buf[8];
    PT_WRITE_PNTR(buf, i);

    io = buf[0]; putc(io, out); // TODO: replace with fwrite
    io = buf[1]; putc(io, out);
    io = buf[2]; putc(io, out);
    io = buf[3]; putc(io, out);
    io = buf[4]; putc(io, out);
    io = buf[5]; putc(io, out);
    io = buf[6]; putc(io, out);
    io = buf[7]; putc(io, out);
}
#endif
void PTD_put_int(FILE * out, unsigned long i) {
    assert(i == (unsigned int) i);
    int io;
    static unsigned char buf[4];
    PT_WRITE_INT(buf, i);
    io = buf[0];
    putc(io, out); // TODO: replace with fwrite
    io = buf[1];
    putc(io, out);
    io = buf[2];
    putc(io, out);
    io = buf[3];
    putc(io, out);
}

void PTD_put_short(FILE * out, unsigned long i) {
    assert(i == (unsigned short) i);
    int io;
    static unsigned char buf[2];
    PT_WRITE_SHORT(buf, i);
    io = buf[0];
    putc(io, out); // TODO: replace with fwrite
    io = buf[1];
    putc(io, out);
}

void PTD_set_object_to_saved_status(POS_TREE * node, long pos, int size) {
    node->flags = 0x20;
    PT_WRITE_PNTR((&node->data), pos);
    if (size < 20) {
        node->flags |= size - sizeof(PT_PNTR);
    } else {
        PT_WRITE_INT((&node->data)+sizeof(PT_PNTR), size);
    }
}

long PTD_write_tip_to_disk(FILE * out, PTM2 * /* ptmain */, POS_TREE * node,
        long pos) {
    int size, cnt;
    putc(node->flags, out); // save type
    size = PT_LEAF_SIZE(node);
    // write 4 bytes when not in stage 2 save mode

    cnt = size - sizeof(PT_PNTR) - 1; // no father; type already saved
#ifdef ARB_64
    fwrite(&node->data + sizeof(PT_PNTR), 0x01, cnt, out); // write name rpos apos
#else
    for (char *data = (&node->data) + sizeof(PT_PNTR); cnt; cnt--) { // write apos rpos name
        int i = (int) (*(data++));
        putc(i, out);
    }
#endif
    PTD_set_object_to_saved_status(node, pos, size);
    pos += size - sizeof(PT_PNTR); // no father
    assert(pos >= 0);
    return pos;
}

int ptd_count_chain_entries(char * entry) {
    int counter = 0;
    long next;
    while (entry) {
        counter++;
        PT_READ_PNTR(entry, next);
        entry = (char *) next;
    }
    assert(counter >= 0);
    return counter;
}

void ptd_set_chain_references(char *entry, char **entry_tab) {
    int counter = 0;
    long next;
    while (entry) {
        entry_tab[counter] = entry;
        counter++;
        PT_READ_PNTR(entry, next);
        entry = (char *) next;
    }
}

bool ptd_write_chain_entries(FILE * out, long *ppos, PTM2 * /* ptmain */,
        char ** entry_tab, int n_entries, int mainapos) { // __ATTR__USERESULT
    bool error = false;
    int lastname = 0;

    while (n_entries > 0 && !error) {
        char *entry = entry_tab[n_entries - 1];
        n_entries--;
        char *rp = entry;
        rp += sizeof(PT_PNTR);

        int name;
        int rpos;
        int apos;
        PT_READ_NAT(rp, name);
        PT_READ_NAT(rp, rpos);
        PT_READ_NAT(rp, apos);
        if (name < lastname) {
            error = true;
        } else {
            static char buffer[100];
            char *wp = buffer;
            wp = PT_WRITE_CHAIN_ENTRY(wp, mainapos, name - lastname, apos,
                    rpos);
            int size = wp - buffer;

            if (1 != fwrite(buffer, size, 1, out)) {
                error = true;
            } else {
                *ppos += size;
                free(entry);
                lastname = name;
            }
        }
    }

    return error;
}

long PTD_write_chain_to_disk(FILE * out, PTM2 *ptmain, POS_TREE * node,
        long pos, bool& error) {
    char *data;
    long oldpos = pos;
    putc(node->flags, out); // save type
    pos++;
    int mainapos;
    data = (&node->data) + ptmain->mode;

    if (node->flags & 1) {
        PT_READ_INT(data, mainapos);
        PTD_put_int(out, mainapos);
        data += 4;
        pos += 4;
    } else {
        PT_READ_SHORT(data, mainapos);
        PTD_put_short(out, mainapos);
        data += 2;
        pos += 2;
    }
    long first_entry;
    PT_READ_PNTR(data, first_entry);
    int n_entries = ptd_count_chain_entries((char *) first_entry);
    {
        char **entry_tab = (char **) calloc(sizeof(char *), n_entries);
        ptd_set_chain_references((char *) first_entry, entry_tab);
        error = ptd_write_chain_entries(out, &pos, ptmain, entry_tab, n_entries,
                mainapos);
        free(entry_tab);
    }
    putc(PT_CHAIN_END, out);
    pos++;
    PTD_set_object_to_saved_status(node, oldpos,
            data + sizeof(PT_PNTR) - (char*) node);
    assert(pos >= 0);
    return pos;
}

void PTD_debug_nodes() {
#ifdef ARB_64
    printf ("Inner Node Statistic:\n");
    printf ("   Single Nodes:   %6i\n", ptstruct.stat.single_node);
    printf ("   Short  Nodes:   %6i\n", ptstruct.stat.short_node);
    printf ("       Chars:      %6i\n", ptstruct.stat.chars);
    printf ("       Shorts:     %6i\n", ptstruct.stat.shorts2);
    printf ("   Int    Nodes:   %6i\n", ptstruct.stat.int_node);
    printf ("       Shorts:     %6i\n", ptstruct.stat.shorts);
    printf ("       Ints:       %6i\n", ptstruct.stat.ints2);
    printf ("   Long   Nodes:   %6i\n", ptstruct.stat.long_node);
    printf ("       Ints:       %6i\n", ptstruct.stat.ints);
    printf ("       Longs:      %6i\n", ptstruct.stat.longs);
    printf ("   maxdiff:        %6li\n", ptstruct.stat.maxdiff);
#else
    printf("Inner Node Statistic:\n");
    printf("   Single Nodes:   %6i\n", ptstruct.stat.single_node);
    printf("   Short  Nodes:   %6i\n", ptstruct.stat.short_node);
    printf("       Chars:      %6i\n", ptstruct.stat.chars);
    printf("       Shorts:     %6i\n", ptstruct.stat.shorts2);
    printf("   Long   Nodes:   %6i\n", ptstruct.stat.long_node);
    printf("       Shorts:     %6i\n", ptstruct.stat.shorts);
    printf("       Longs:      %6i\n", ptstruct.stat.longs); // "longs" are actually 32 bit ints
#endif
}

long PTD_write_node_to_disk(FILE * out, PTM2 *ptmain, POS_TREE * node,
        long *r_poss, long pos) {
    int i, size; // Save node after all descendends are already saved
    POS_TREE *sons;

    long max_diff = 0;
    int lasti = 0;
    int count = 0;
    int mysize;

    size = PT_EMPTY_NODE_SIZE;
    mysize = PT_EMPTY_NODE_SIZE;

    for (i = PT_QU; i < PT_B_MAX; i++) { // free all sons
        sons = PT_read_son_stage_1(ptmain, node, (enum PT_bases_enum) i);
        if (sons) {
            // int memsize; // TODO: UNUSED?
            long diff = pos - r_poss[i];
            assert(diff >= 0);
            if (diff > max_diff) {
                max_diff = diff;
                lasti = i;
#ifdef ARB_64
                if (max_diff > ptstruct.stat.maxdiff) {
                    ptstruct.stat.maxdiff = max_diff;
                }
#endif
            }
            mysize += sizeof(PT_PNTR);
            if (PT_GET_TYPE(sons) != PT_NT_SAVED) {
                printf("Internal Error: Son not saved");
                exit(EXIT_FAILURE);
            }
            //if ((sons->flags & 0xf) == 0) { // TODO: UNUSED?
            //    PT_READ_INT((&sons->data)+sizeof(PT_PNTR), memsize);
            //} else {
            //    memsize = (sons->flags & 0xf) + sizeof(PT_PNTR);
            //}
            free(sons);
            count++;
        }
    }
    if ((count == 1) && (max_diff <= 9) && (max_diff != 2)) { // nodesingle
        if (max_diff > 2)
            max_diff -= 2;
        else
            max_diff -= 1;
        long flags = 0xc0 | lasti | (max_diff << 3);
        putc((int) flags, out);
        ptstruct.stat.single_node++;
    } else { // multinode
        putc(node->flags, out);
        int flags2 = 0;
        int level;
#ifdef ARB_64
        if (max_diff > 0xffffffff) { // long node
            // Warning: max_diff > 0xffffffff is not tested.
            flags2 |= 0x40;
            level = 0xffffffff;
            ptstruct.stat.long_node++;
        }
        else if (max_diff > 0xffff) { // int node
            flags2 |= 0x80;
            level = 0xffff;
            ptstruct.stat.int_node++;
        }
        else { // short node
            level = 0xff;
            ptstruct.stat.short_node++;
        }
#else
        if (max_diff > 0xffff) {
            flags2 |= 0x80;
            level = 0xffff;
            ptstruct.stat.long_node++;
        } else {
            max_diff = 0;
            level = 0xff;
            ptstruct.stat.short_node++;
        }
#endif
        for (i = PT_QU; i < PT_B_MAX; i++) { // set the flag2
            if (r_poss[i]) {
                /* u */long diff = pos - r_poss[i];
                assert(diff >= 0);
                if (diff > level)
                    flags2 |= 1 << i;
            }
        }
        putc(flags2, out);
        size++;
        for (i = PT_QU; i < PT_B_MAX; i++) { // write the data
            if (r_poss[i]) {
                /* u */long diff = pos - r_poss[i];
                assert(diff >= 0);
#ifdef ARB_64
                if (max_diff > 0xffffffff) { // long long / int  (bit[6] in flags2 is set; bit[7] is unset)
                    // Warning: max_diff > 0xffffffff is not tested.
                    if (diff>level) { // long long (64 bit)  (bit[i] in flags2 is set)
                        PTD_put_longlong(out, diff);
                        size += 8;
                        ptstruct.stat.longs++;
                    }
                    else { // int              (bit[i] in flags2 is unset)
                        PTD_put_int(out, diff);
                        size += 4;
                        ptstruct.stat.ints++;
                    }
                }
                else if (max_diff > 0xffff) { // int/short        (bit[6] in flags2 is unset; bit[7] is set)
                    if (diff>level) { // int              (bit[i] in flags2 is set)
                        PTD_put_int(out, diff);
                        size += 4;
                        ptstruct.stat.ints2++;
                    }
                    else { // short            (bit[i] in flags2 is unset)
                        PTD_put_short(out, diff);
                        size += 2;
                        ptstruct.stat.shorts++;
                    }
                }
                else { // short/char       (bit[6] in flags2 is unset; bit[7] is unset)
                    if (diff>level) { // short            (bit[i] in flags2 is set)
                        PTD_put_short(out, diff);
                        size += 2;
                        ptstruct.stat.shorts2++;
                    }
                    else { // char             (bit[i] in flags2 is unset)
                        putc((int)diff, out);
                        size += 1;
                        ptstruct.stat.chars++;
                    }
                }
#else
                if (max_diff) { // int/short (bit[7] in flags2 set)
                    if (diff > level) { // int
                        PTD_put_int(out, diff);
                        size += 4;
                        ptstruct.stat.longs++;
                    } else { // short
                        PTD_put_short(out, diff);
                        size += 2;
                        ptstruct.stat.shorts++;
                    }
                } else { // short/char  (bit[7] in flags2 not set)
                    if (diff > level) { // short
                        PTD_put_short(out, diff);
                        size += 2;
                        ptstruct.stat.shorts2++;
                    } else { // char
                        putc((int) diff, out);
                        size += 1;
                        ptstruct.stat.chars++;
                    }
                }
#endif
            }
        }
    }

    PTD_set_object_to_saved_status(node, pos, mysize);
    pos += size - sizeof(PT_PNTR); // no father
    assert(pos >= 0);
    return pos;
}

long PTD_write_leafs_to_disk(FILE * out, PTM2 *ptmain, POS_TREE * node,
        long pos, long *pnodepos, int *pblock, bool &error) {
    // returns new pos when son is written 0 otherwise
    // pnodepos is set to last object

    POS_TREE *sons;
    // long r_pos, r_poss[PT_B_MAX], son_size[PT_B_MAX], o_pos; // TODO: son_size UNUSED?
    long r_pos, r_poss[PT_B_MAX], o_pos;
    int block[10]; // TODO: check why we allocate 10 ints when only block[0] is used
    int i;
    PT_NODE_TYPE type = PT_read_type(node);

    if (type == PT_NT_SAVED) { // already saved
        long father;
        PT_READ_PNTR((&node->data), father);
        *pnodepos = father;
        return 0;
    } else if (type == PT_NT_LEAF) {
        *pnodepos = pos;
        pos = PTD_write_tip_to_disk(out, ptmain, node, pos);
    } else if (type == PT_NT_CHAIN) {
        *pnodepos = pos;
        pos = PTD_write_chain_to_disk(out, ptmain, node, pos, error);
    } else if (type == PT_NT_NODE) {
        block[0] = 0;
        o_pos = pos;
        for (i = PT_QU; i < PT_B_MAX && !error; i++) { // save all sons
            sons = PT_read_son_stage_1(ptmain, node, (enum PT_bases_enum) i);
            r_poss[i] = 0;
            if (sons) {
                r_pos = PTD_write_leafs_to_disk(out, ptmain, sons, pos,
                        &(r_poss[i]), &(block[0]), error);
                if (r_pos > pos) { // really saved ????
                    // son_size[i] = r_pos - pos; // TODO: UNUSED?
                    pos = r_pos;
                }
                //else {
                //son_size[i] = 0; // TODO: UNUSED?
                //}
                //}
                //else {
                //son_size[i] = 0; // TODO: UNUSED?
            }
        }
        if (block[0]) { // son wrote a block
            *pblock = 1;
        } else if (pos - o_pos > PT_BLOCK_SIZE) {
            // a block is written
            *pblock = 1;
        } else { // now i can write my data
            *pnodepos = pos;
            if (!error)
                pos = PTD_write_node_to_disk(out, ptmain, node, r_poss, pos);
        }
    }
    assert(pos >= 0 || error);
    return pos;
}

char *GB_map_FILE(FILE *in, int writeable) {
    int fi = fileno(in);
    struct stat st;
    if (fstat(fi, &st)) {
        return NULL;
    }
    size_t size = st.st_size;

    char *buffer = NULL;
    if (size <= 0) {
        printf("GB_map_file: sorry file not found");
        return NULL;
    }
#ifdef _WIN32
    // TODO: Use MapViewOfFile(...) to do something equivalent to mmap(...)!
    buffer = (char*) malloc(size);
    if (buffer == NULL) {
        printf("GB_map_file: Error: Out of Memory: mmap failed");
        return NULL;
    }
    fread(buffer, 1, size, in);
#else
    if (writeable) {
        buffer = (char*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                fi, 0);
    } else {
        buffer = (char*) mmap(NULL, size, PROT_READ, MAP_SHARED, fi, 0);
    }
    if (buffer == MAP_FAILED) {
        printf("GB_map_file: Error: Out of Memory: mmap failed");
        return NULL;
    }
#endif
    return buffer;
}

char *GB_map_file(const char *path, int writeable) {
    FILE *in;
    char *buffer;
    in = fopen(path, "r");
    if (!in) {
        printf("GB_map_file: sorry file '%s' not readable", path);
        return NULL;
    }
    buffer = GB_map_FILE(in, writeable);
    fclose(in);
    return buffer;
}

// --------------------------------------
//      functions for stage 2-3: load

bool PTD_read_leafs_from_disk(const char *fname, PTM2 *ptmain,
        POS_TREE **pnode) { // __ATTR__USERESULT
    bool error = false;
    char *buffer = GB_map_file(fname, 0);

    if (!buffer) {
        error = true;
    } else {
        FILE *in = fopen(fname, "r");
        int fi = fileno(in);
        struct stat st;
        if (fstat(fi, &st)) {
            return true;
        }
        size_t size = st.st_size;
        fclose(in);
        char *main = &(buffer[size - 4]);

        long i;
        PT_READ_INT(main, i);
#ifdef ARB_64
        if (i == 0xffffffff) { // 0xffffffff signalizes that "last_obj" is stored
            main -= 8;// in the previous 8 byte (in a long long)
            PT_READ_PNTR(main, i);// this search tree can only be loaded at 64 Bit
            // TODO: UNUSED: big_db = true;
        }
#else
        bool big_db = false;
        if (i < 0) {
            assert(i == -1);
            // aka 0xffffffff
            big_db = true; // not usable in 32-bit version (fails below)
        }
        assert(i <= INT_MAX);
#endif

        // try to find info_area
        main -= 2;
        short info_size;
        PT_READ_SHORT(main, info_size);
#ifndef ARB_64
        bool info_detected = false;
#endif
        if (info_size > 0 && info_size < (main - buffer)) { // otherwise impossible size
            main -= info_size;

            long magic;
            int version;

            PT_READ_INT(main, magic);
            main += 4;
            PT_READ_CHAR(main, version);
            main++;

            assert(PT_SERVER_MAGIC > 0 && PT_SERVER_MAGIC < INT_MAX);

            if (magic == PT_SERVER_MAGIC) {
#ifndef ARB_64
                info_detected = true;
#endif
                if (version > PT_SERVER_VERSION) {
                    error = true;
                }
            }
        }

#ifndef ARB_64
        // 32bit version:
        if (!error && big_db) {
            error = true;
        }
        if (!error && !info_detected) {
            printf("Warning: ptserver DB has old format (no problem)\n");
        }
#endif
        if (!error) {
            assert(i >= 0);

            *pnode = (POS_TREE *) (i + buffer);
            ptmain->mode = 0;
            ptmain->data_start = buffer;
        }
    }

    return error;
}

} /* namespace minipt */
