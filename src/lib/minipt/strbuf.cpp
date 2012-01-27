/*!
 * MiniPT Search Index Library
 *
 * This is a functionally reduced version of the ARB PT-Server.
 * It was specially adapted for CaSSiS. (faster; less memory consumption)
 *
 * Modified by Kai Christian Bader <mail@kaibader.de>
 *
 * Original source: arb_strbuf.cxx
 * Institute of Microbiology (Technical University Munich)
 * http://www.arb-home.de/
 */

#include "strbuf.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

namespace minipt {

void GBS_strstruct::set_pos(size_t toPos) {
    pos = toPos;
    data[pos] = 0;
}

void GBS_strstruct::inc_pos(size_t inc) {
    set_pos(pos + inc);
}

GBS_strstruct::GBS_strstruct() :
                data(NULL), buffer_size(0), pos(0) {
}

GBS_strstruct::GBS_strstruct(size_t buffersize) :
                data(NULL), buffer_size(0), pos(0) {
    alloc_mem(buffersize);
}

GBS_strstruct::~GBS_strstruct() {
    free(data);
}

size_t GBS_strstruct::get_buffer_size() const {
    return buffer_size;
}

size_t GBS_strstruct::get_position() const {
    return pos;
}

const char *GBS_strstruct::get_data() const {
    return data;
}

char *GBS_strstruct::release_mem(size_t& size) {
    char *result = data;
    size = buffer_size;
    buffer_size = 0;
    data = 0;
    return result;
}

void GBS_strstruct::reset_pos() {
    set_pos(0);
}

void GBS_strstruct::assign_mem(char *block, size_t blocksize) {
    free(data);

    assert(block && blocksize > 0);

    data = block;
    buffer_size = blocksize;

    reset_pos();
}

void GBS_strstruct::reassign_mem(GBS_strstruct& from) {
    size_t size;
    char *block = from.release_mem(size);

    assign_mem(block, size);
}

void GBS_strstruct::alloc_mem(size_t blocksize) {
    assert(blocksize > 0);
    assert(!data);

    assign_mem((char*) malloc(blocksize), blocksize);
}

void GBS_strstruct::realloc_mem(size_t newsize) {
    if (!data)
        alloc_mem(newsize);
    else {
        // cppcheck-suppress memleakOnRealloc
        data = (char*) realloc(data, newsize);
        buffer_size = newsize;

        assert(pos < newsize);
    }
}

void GBS_strstruct::ensure_mem(size_t needed_size) {
    // ensures insertion of 'needed_size' bytes is ok
    size_t whole_needed_size = pos + needed_size + 1;
    if (buffer_size < whole_needed_size) {
        size_t next_size = (whole_needed_size * 3) >> 1;
        realloc_mem(next_size);
    }
}

void GBS_strstruct::cut_tail(size_t byte_count) {
    set_pos(pos < byte_count ? 0 : pos - byte_count);
}

void GBS_strstruct::put(char c) {
    ensure_mem(1);
    data[pos] = c;
    inc_pos(1);
}

void GBS_strstruct::nput(char c, size_t count) {
    ensure_mem(count);
    memset(data + pos, c, count);
    inc_pos(count);
}

void GBS_strstruct::ncat(const char *from, size_t count) {
    if (count) {
        ensure_mem(count);
        memcpy(data + pos, from, count);
        inc_pos(count);
    }
}

void GBS_strstruct::cat(const char *from) {
    ncat(from, strlen(from));
}

void GBS_strstruct::vnprintf(size_t maxlen, const char *templat,
        va_list& parg) {
    ensure_mem(maxlen);

    char *buffer = data + pos;
    int printed;

#ifdef LINUX
    printed = vsnprintf(buffer, maxlen, templat, parg);
#else
    printed = vsprintf(buffer, templat, parg);
#endif

    assert(printed >= 0 && (size_t) printed <= maxlen);
    inc_pos(printed);
}

void GBS_strstruct::nprintf(size_t maxlen, const char *templat, ...) {
    va_list parg;
    va_start(parg, templat);
    vnprintf(maxlen, templat, parg);
}

static GBS_strstruct last_used;

GBS_strstruct *GBS_stropen(long init_size) {
    /*! create a new memory file
     * @param init_size  estimated used size
     */

    GBS_strstruct *strstr = new GBS_strstruct;

    assert(init_size > 0);

    if (last_used.get_buffer_size() >= (size_t) init_size) {
        strstr->reassign_mem(last_used);

        static short oversized_counter = 0;

        if ((size_t) init_size * 10 < strstr->get_buffer_size())
            oversized_counter++;
        else
            oversized_counter = 0;

        if (oversized_counter > 10) { // was oversized more than 10 times -> realloc
            size_t dummy;
            free(strstr->release_mem(dummy));
            strstr->alloc_mem(init_size);
        }
    } else {
        strstr->alloc_mem(init_size);
    }

    return strstr;
}

char *GBS_strclose(GBS_strstruct *strstr) {
    // returns a char* copy of the memory file

    size_t length = strstr->get_position();
    char *str = (char*) malloc(length + 1);

    memcpy(str, strstr->get_data(), length + 1); // copy with 0
    GBS_strforget(strstr);

    return str;
}

void GBS_strforget(GBS_strstruct *strstr) {
    size_t last_bsize = last_used.get_buffer_size();
    size_t curr_bsize = strstr->get_buffer_size();

    if (last_bsize < curr_bsize) { // last_used is smaller -> keep this
        last_used.reassign_mem(*strstr);
    }
    delete strstr;
}

char *GBS_mempntr(GBS_strstruct *strstr) {
    // returns the memory file (with write access)
    return (char*) strstr->get_data();
}

long GBS_memoffset(GBS_strstruct *strstr) {
    // returns the offset into the memory file
    return strstr->get_position();
}

void GBS_str_cut_tail(GBS_strstruct *strstr, size_t byte_count) {
    // Removes byte_count characters at the tail of a memfile
    strstr->cut_tail(byte_count);
}

void GBS_strncat(GBS_strstruct *strstr, const char *ptr, size_t len) {
    /* append some bytes string to strstruct
     * (caution : copies zero byte and mem behind if used with wrong len!)
     */
    strstr->ncat(ptr, len);
}

void GBS_strcat(GBS_strstruct *strstr, const char *ptr) {
    // append string to strstruct
    strstr->cat(ptr);
}

void GBS_strnprintf(GBS_strstruct *strstr, long maxlen, const char *templat,
        ...) {
    va_list parg;
    va_start(parg, templat);
    strstr->vnprintf(maxlen + 2, templat, parg);
}

void GBS_chrcat(GBS_strstruct *strstr, char ch) {
    strstr->put(ch);
}

void GBS_chrncat(GBS_strstruct *strstr, char ch, size_t n) {
    strstr->nput(ch, n);
}

void GBS_intcat(GBS_strstruct *strstr, long val) {
    char buffer[100];
    long len = sprintf(buffer, "%li", val);
    GBS_strncat(strstr, buffer, len);
}

void GBS_floatcat(GBS_strstruct *strstr, double val) {
    char buffer[100];
    long len = sprintf(buffer, "%f", val);
    GBS_strncat(strstr, buffer, len);
}

} /* namespace minipt */
