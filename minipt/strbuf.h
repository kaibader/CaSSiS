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

#ifndef MINIPT_STRBUF_H
#define MINIPT_STRBUF_H

#include <cstddef>
#include <cstdarg>

namespace minipt {

class GBS_strstruct {
private:
    GBS_strstruct(const GBS_strstruct&);
    GBS_strstruct& operator=(const GBS_strstruct&);
    char *data;
    size_t buffer_size;
    size_t pos;
    void set_pos(size_t toPos);
    void inc_pos(size_t inc);
public:
    GBS_strstruct();
    GBS_strstruct(size_t buffersize);
    ~GBS_strstruct();
    size_t get_buffer_size() const;
    size_t get_position() const;
    const char *get_data() const;
    char *release_mem(size_t& size);
    void reset_pos();
    void assign_mem(char *block, size_t blocksize);
    void reassign_mem(GBS_strstruct& from);
    void alloc_mem(size_t blocksize);
    void realloc_mem(size_t newsize);
    void ensure_mem(size_t needed_size);
    void cut_tail(size_t byte_count);
    void put(char c);
    void nput(char c, size_t count);
    void ncat(const char *from, size_t count);
    void cat(const char *from);
    void vnprintf(size_t maxlen, const char *templat, va_list& parg);
    void nprintf(size_t maxlen, const char *templat, ...);
};

GBS_strstruct *GBS_stropen(long init_size);
char *GBS_strclose(GBS_strstruct *strstr);
void GBS_strforget(GBS_strstruct *strstr);
char *GBS_mempntr(GBS_strstruct *strstr);
long GBS_memoffset(GBS_strstruct *strstr);
void GBS_str_cut_tail(GBS_strstruct *strstr, size_t byte_count);
void GBS_strncat(GBS_strstruct *strstr, const char *ptr, size_t len);
void GBS_strcat(GBS_strstruct *strstr, const char *ptr);
void GBS_strnprintf(GBS_strstruct *strstr, long maxlen, const char *templat,
        ...);
void GBS_chrcat(GBS_strstruct *strstr, char ch);
void GBS_chrncat(GBS_strstruct *strstr, char ch, size_t n);
void GBS_intcat(GBS_strstruct *strstr, long val);
void GBS_floatcat(GBS_strstruct *strstr, double val);

} /* namespace minipt */

#endif /* MINIPT_STRBUF_H */
