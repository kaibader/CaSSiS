/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: probe.h
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#ifndef MINIPT_PROBE_H
#define MINIPT_PROBE_H

#include <list>
#include <set>
#include <cstring>
#include <cstdlib>

namespace minipt {

#define PT_SERVER_MAGIC   0x32108765                // pt server identifier
#define PT_SERVER_VERSION 2                         // version of pt server database (no versioning till 2009/05/13)
#define PT_MAX_MATCHES     1024*1024
// #define PT_MAX_IDENTS      10000
#define PT_POS_TREE_HEIGHT 80
#define PT_POS_SECURITY    9
#define MIN_PROBE_LENGTH   9

enum PT_MATCH_TYPE {
    PT_MATCH_TYPE_INTEGER = 0,
    PT_MATCH_TYPE_WEIGHTED_PLUS_POS = 1,
    PT_MATCH_TYPE_WEIGHTED = -1
};

extern unsigned long physical_memory;

typedef enum PT_bases_enum {
    PT_QU = 0, PT_N = 1, PT_A, PT_C, PT_G, PT_T, PT_B_MAX, PT_B_UNDEF
} PT_BASES;

typedef enum enum_PT_NODE_TYPE {
    PT_NT_LEAF = 0, PT_NT_CHAIN = 1, PT_NT_NODE = 2, PT_NT_SINGLE_NODE = 3, // stage 3
    PT_NT_SAVED = 3, // stage 1+2
    PT_NT_UNDEF = 4
} PT_NODE_TYPE;

typedef struct POS_TREE_struct {
    unsigned char flags;
    char data;
} POS_TREE;

typedef struct PTMM_struct {
    char *data_start; // points to start of data
    int stage1;
    int stage2;
    int stage3;
    int mode;
} PTM2;

//// ---------------------
////      Probe search

struct probe_statistic {
    int match_count; // Counter for matches
    double rel_match_count; // match_count / (seq_len - probe_len + 1)
};

struct PT_probematch { // the probe match list
    PT_probematch *next;
    PT_probematch *last;
    //
    int name; // ID of the matched sequence
    int b_pos; // pos of probe
    int g_pos; // pos of probe (gene relative)
    int rpos; // relative pos of probe
    int mismatches; // number of mismatches
    double wmismatches; // number of weighted mismatches
    int N_mismatches; // number of 'N' mismatches
    char *sequence; // path of probe
    int reversed; // reversed probe matches
};

struct ProbeDataStruct { // every taxa's own data
    char *data; // sequence
    long checksum;
    int size;

    // probe design
    int is_group; // -1: nevermind, 0: no group, 1: group

    // probe design (match)
    PT_probematch *match; // best hit for PT_new_design

    // find family
    probe_statistic stat;

    // External ID. Equals internal index position by default.
    unsigned int id;

    int next;
};

struct probe_statistic_struct {
#ifdef ARB_64
    int cut_offs; // statistic of chains
    int single_node;
    int short_node;
    int int_node;
    int long_node;
    int longs;
    int ints;
    int ints2;
    int shorts;
    int shorts2;
    int chars;
    long maxdiff;
#else
    int cut_offs; // statistic of chains
    int single_node;
    int short_node;
    int long_node;
    int longs;
    int shorts;
    int shorts2;
    int chars;
#endif

};

extern struct MiniPTStruct {
    struct ProbeDataStruct *data; // The internal sequence database
    unsigned int data_max_size; // Max in-memory-size of the sequence database
    unsigned int data_count; // Number of entries in the sequence database
    int max_size; // maximum sequence length
    long char_count; // number of all characters (only 'acgtuACGTU')
    int mismatches; // chain handle in match
    double wmismatches;
    int N_mismatches;
    int w_N_mismatches[(PT_POS_TREE_HEIGHT + PT_POS_SECURITY + 1)]; //
    int reversed; // tell the matcher whether probe is reversed
    double *pos_to_weight; // position to weight
    char complement[256]; // complement
    int deep; // for probe matching
    int height;
    int length;
    int sort_by;
    char *probe; // probe design + chains
    char *main_probe;
    char *server_name; // name of this server
    POS_TREE *pt;
    PTM2 *ptmain;
    probe_statistic_struct stat;
    // Number of sequences that were (already) imported into the database.
    unsigned int imported_sequences_counter;
} ptstruct;

} /* namespace minipt */

#endif /* MINIPT_PROBE_H */
