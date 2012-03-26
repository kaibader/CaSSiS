/*!
 * MultiFASTA formatted file reader
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
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

#include "fasta.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace FASTA {

/*!
 * The initial size of the Fasta file handles buffer
 */
const size_t INITIAL_BUFFER_SIZE = 1024 * 16;

/***************************************************************************
 * Sequence class methods come here...
 ***************************************************************************/

/*!
 * Constructor.
 */
Sequence::Sequence() :
                m_header(NULL), m_name(NULL), m_sequence(NULL), m_length(0), m_type(
                        UNDEF) {
}

/*!
 * Destructor
 */
Sequence::~Sequence() {
    free(m_header);
    free(m_name);
    free(m_sequence);
}

/***************************************************************************
 * FastaFile implementation class methods come here...
 ***************************************************************************/

/*!
 * Implementation of the FastaFile class
 */
class FileImpl {
public:
    FileImpl();
    FileImpl(const char *filename, Type type);
    ~FileImpl();
    bool open(const char *filename, Type type);
    bool isOpen();
    bool atEOF();
    void close();
    Sequence *getSequence();
private:
    FileImpl(const FileImpl&); // Hide copy constructor
    FileImpl& operator=(FileImpl); // Hide copy assignment operator
    size_t doubleSeqBufSize();
    void createFilterMask();
    void whiteListFilter(char *sequence, const unsigned char *mask);
    void cleanUp(char *string, const char *rm);
    FILE *m_fd;
    Type m_type;
    char *m_read_buf;
    size_t m_read_buf_size;
    char *m_seq_buf;
    size_t m_seq_buf_size;
    unsigned char *m_filter_mask;
};

/*!
 * Implementation of the FastaFile class -- Constructor, including parameters
 *
 * @param filename Fasta file name
 * @param type Seqence type (DNA,RNA,AMINO)
 */
FileImpl::FileImpl(const char *filename, Type type) :
                m_fd(NULL), m_type(type), m_read_buf(NULL), m_read_buf_size(0), m_seq_buf(
                        NULL), m_seq_buf_size(0), m_filter_mask(NULL) {
    open(filename, type);
}

/*!
 * Implementation of the FastaFile class -- Destructor
 */
FileImpl::~FileImpl() {
    close();
}

/*!
 * Implementation of the FastaFile class -- File open method
 *
 * @param filename Fasta file name
 * @param type Seqence type (DNA,RNA,AMINO)
 * @return true, if the file was successfully opened
 */
bool FileImpl::open(const char *filename, Type type) {
    // Open the Fasta file with read access
    m_fd = NULL;
    if ((m_fd = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Error while opening Fasta file: %s\n", filename);
        return false;
    }

    // Set the sequences type and add a filter mask
    m_type = type;
    createFilterMask();

    // Create a read buffer
    m_read_buf = (char *) malloc(INITIAL_BUFFER_SIZE);
    if (!m_read_buf) {
        fprintf(stderr,
                "Unable to create a read buffer for the Fasta file: %s\n",
                filename);
        fclose(m_fd);
        m_fd = NULL;
        return false;
    }
    m_read_buf[0] = 0;
    m_read_buf[INITIAL_BUFFER_SIZE - 1] = 0;
    m_read_buf_size = INITIAL_BUFFER_SIZE;

    // Create a sequence buffer
    m_seq_buf = NULL;
    m_seq_buf_size = 0;
    if (doubleSeqBufSize() == 0) {
        fprintf(stderr,
                "Unable to create a sequence buffer for the Fasta file: %s\n",
                filename);
        fclose(m_fd);
        m_fd = NULL;
        return false;
    }

    // Fetch the first line: A Fasta file should start with a '>' character
    // TODO: What, if the header is longer than the buffers' size?
    if (!fgets(m_read_buf, INITIAL_BUFFER_SIZE - 1, m_fd)) {
        fprintf(stderr, "Error while trying to read the file: %s\n", filename);
        fclose(m_fd);
        m_fd = NULL;
        return false;
    }
    if (m_read_buf[0] != '>') {
        fprintf(stderr, "Is this really a Fasta file...? File: %s\n", filename);
        fclose(m_fd);
        m_fd = NULL;
        return false;
    }

    return true;
}

/*!
 * Implementation of the FastaFile class -- File open indicator
 *
 * @return true, if a Fasta file was successfully opened
 */
bool FileImpl::isOpen() {
    if (m_fd)
        return true;
    return false;
}

/*!
 * Implementation of the FastaFile class -- End of file indicator
 *
 * @return true, if end of file is reached or no file is opened
 */
bool FileImpl::atEOF() {
    if (!m_fd)
        return true;
    return (feof(m_fd) != 0);
}

/*!
 * Implementation of the FastaFile class -- Close file
 */
void FileImpl::close() {
    // Close file handle
    if (m_fd) {
        fclose(m_fd);
        m_fd = NULL;
    }

    // Free the buffers...
    if (m_read_buf) {
        free(m_read_buf);
        m_read_buf = NULL;
        m_read_buf_size = 0;
    }
    if (m_seq_buf) {
        free(m_seq_buf);
        m_seq_buf = NULL;
        m_seq_buf_size = 0;
    }

    // Clear filter mask
    if (m_filter_mask) {
        free(m_filter_mask);
        m_filter_mask = NULL;
    }
}

/*!
 * FastaFile class -- Fetch the next sequence
 *
 * @return Sequence object (pointer), if successful. Otherwise NULL.
 */
Sequence *FileImpl::getSequence() {
    if (!m_fd || !m_read_buf || feof(m_fd))
        return NULL;

    // Just to be on the safe side...
    m_read_buf[m_read_buf_size - 1] = 0;

    // We should already have read a header into our buffer...
    if (m_read_buf[0] != '>') {
        fprintf(stderr, "Error while reading header for a given sequence.\n");
        return NULL;
    }

    // Remove unwanted characters
    cleanUp(m_read_buf, "\r\n");

    // Create Sequence and return it
    Sequence *sequence = new Sequence();

    // Copy header into the sequence struct.
    // First '>' character is stripped.
    size_t len = strlen(m_read_buf);

    if (len) {
        sequence->m_header = (char *) malloc(len);
        strncpy(sequence->m_header, m_read_buf + 1, len - 1);
        sequence->m_header[len - 1] = 0; // TODO: Is this necessary?
    }

    // Add a reference ID
    // (Substring up to first space, if available.)
    len = strcspn(sequence->m_header, " ");
    if (len > 0) {
        sequence->m_name = (char *) malloc(len + 1);
        memcpy(sequence->m_name, sequence->m_header, len);
        sequence->m_name[len] = 0; // TODO: Is this necessary?
    }

    // Fetch the sequence...
    size_t seq_pos = 0;
    while (true) {
        // fgets returns NULL when reaching EOF.
        char *ptr = fgets(m_read_buf, m_read_buf_size - 1, m_fd);
        if (ptr == NULL) {
            break;
        }

        m_read_buf[m_read_buf_size - 1] = 0;

        // Return sequence, if we have reached another header.
        if (m_read_buf[0] == '>')
            break;

        // Process the sequence line, if it isn't a comment.
        if (m_read_buf[0] != ';') {
            // Increase the sequence buffer size, if necessary.
            len = strlen(m_read_buf);
            if (m_seq_buf_size - 1 < seq_pos + len)
                if (doubleSeqBufSize() == 0) {
                    fprintf(stderr,
                            "Error increasing the sequence buffer size.\n");
                    delete sequence;
                    return NULL;
                }

            // Copy read buffer to sequence string
            memcpy(m_seq_buf + seq_pos, m_read_buf, len + 1);
            seq_pos += len;
        }

        // Another check for the EOF.
        if (feof(m_fd))
            break;
    }

    // Convert all sequence characters to upper case...
    char *ptr = m_seq_buf;
    while (*ptr) {
        if (*ptr > 96 && *ptr < 123)
            *ptr = *ptr - 32;
        ++ptr;
    }

    // Only allow certain characters in the sequence string
    whiteListFilter(m_seq_buf, m_filter_mask);

    sequence->m_length = strlen(m_seq_buf);
    sequence->m_sequence = (char *) malloc(sequence->m_length + 1);
    if (!sequence->m_sequence) {
        fprintf(stderr, "Error allocating memory for sequence data.\n");
        delete sequence;
        return NULL;
    }
    memcpy(sequence->m_sequence, m_seq_buf, sequence->m_length);
    sequence->m_sequence[sequence->m_length] = 0; // TODO: Is this necessary?

    return sequence;
}

/*!
 * FastaFile class -- Doubles the buffer size
 */
size_t FileImpl::doubleSeqBufSize() {
    // Determine the new buffer size
    size_t new_seq_buf_size = 2 * m_seq_buf_size;
    if (new_seq_buf_size == 0)
        new_seq_buf_size = INITIAL_BUFFER_SIZE;

    // Allocate the new buffers memory
    char *old_seq_buf = m_seq_buf;
    m_seq_buf = (char *) malloc(new_seq_buf_size);
    if (!m_seq_buf) {
        m_seq_buf = old_seq_buf;
        return 0;
    }

    // Copy old buffer content into the new one
    memcpy(m_seq_buf, old_seq_buf, m_seq_buf_size);
    m_seq_buf_size = new_seq_buf_size;

    // Delete old buffer
    if (old_seq_buf)
        free(old_seq_buf);

    return new_seq_buf_size;
}

/*!
 * FastaFile class -- Removes unwanted characters from a string
 *
 * TODO: Re-implement a cost-effective variant of this method
 */
void FileImpl::cleanUp(char *string, const char *rm) {
    if (!string || !rm)
        return;

    char *ptr = string; // Start with the first char of the string
    size_t len = strlen(string); // At first, string length = reference

    // Fetch number of bytes that are legal chars
    size_t skip = strcspn(ptr, rm);
    while (*ptr && skip < len) {
        // Ignore legal characters
        ptr += skip;
        len -= skip;

        // Skip illegal character, copy rest of string (including '0')
        memmove(ptr, ptr + 1, len);
        skip = strcspn(ptr, rm);
    }
}

/*!
 * FastaFile class --
 * Creates a sequence filter mask based on the sequence type setting.
 */
void FileImpl::createFilterMask() {
    if (m_filter_mask != NULL) {
        free(m_filter_mask);
        m_filter_mask = NULL;
    }

    // Allowed characters, according to the IUPAC code.
    // --> "RNA", "DNA", "AMINO"
    static const char *whitelist[] = { "ACGURYSWKMBDHVN", "ACGTRYSWKMBDHVN",
            "ACDEFGHIKLMNPQRSTVWY" };

    unsigned char *array = (unsigned char *) calloc(256, sizeof(char));
    if (!array)
        return;

    // Allow all characters if we have no defined type.
    if (m_type == UNDEF) {
        unsigned char i = 255;
        while (i) {
            array[i] = i;
            --i;
        }
        return;
    }

    // Pointer to the whitelist characters...
    const unsigned char *wl_ptr = (const unsigned char *) whitelist[m_type];

    // Mark the respective characters in the array
    for (unsigned int i = 0; i < strlen((const char *) wl_ptr); ++i) {
        unsigned char c = wl_ptr[i];
        array[c] = c; // Upper case letters.
        array[c + 'a' - 'A'] = c; // Lower case letters.
    }

    // Special cases:
    array[(unsigned char) '-'] = '-'; // Alignment gap.
    array[(unsigned char) '.'] = '.'; // Unofficial gap character.
    if (m_type == DNA) {
        array[(unsigned char) 'U'] = 'T'; // Convert Uracil to Thymine.
        array[(unsigned char) 'u'] = 'T';
    } else if (m_type == RNA) {
        array[(unsigned char) 'T'] = 'U'; // Convert Thymine to Uracil.
        array[(unsigned char) 't'] = 'U';
    }

    m_filter_mask = array;
    return;
}

/*!
 * FastaFile class -- Filters a sequence string for allowed characters.

 * @param sequence Sequence to be filtered
 * @param whitelist Allowed characters
 */
void FileImpl::whiteListFilter(char *sequence, const unsigned char *mask) {
    if (!sequence || !mask)
        return;

    unsigned char *r_ptr = (unsigned char *) sequence;
    unsigned char r_char = 0;
    unsigned char *w_ptr = (unsigned char *) sequence;

    while ((r_char = *r_ptr) != 0) {
        unsigned char c = mask[r_char];
        if (c) {
            *w_ptr = c;
            w_ptr++;
        }
        r_ptr++;
    }
    *w_ptr = 0;
}

/***************************************************************************
 * FastaFile class methods come here...
 ***************************************************************************/

/*!
 * FastaFile class -- Constructor, including parameters
 *
 * @param filename Fasta file name
 * @param type Seqence type (DNA,RNA,AMINO)
 */
File::File(const char *filename, Type type) :
                m_impl(new FileImpl(filename, type)) {
}

/*!
 * FastaFile class -- Destructor
 */
File::~File() {
    delete m_impl;
}

///*!
// * FastaFile class -- File open method
// *
// * @param filename Fasta file name
// * @param type Seqence type (DNA,RNA,AMINO)
// * @return true, if the file was successfully opened
// */
//bool FastaFile::open(const char *filename, enum SeqType_old type) {
//    return m_impl->open(filename, type);
//}

/*!
 * FastaFile class -- File open indicator
 *
 * @return true, if a Fasta file was successfully opened
 */
bool File::isOpen() {
    return m_impl->isOpen();
}

/*!
 * FastaFile class -- End of file indicator
 *
 * @return true, if end of file is reached or no file is opened
 */
bool File::atEOF() {
    return m_impl->atEOF();
}

/*!
 * FastaFile class -- Close file
 */
void File::close() {
    m_impl->close();
}

/*!
 * FastaFile class -- Fetch the next sequence
 *
 * @return Sequence object (pointer), if successful. Otherwise NULL.
 */
Sequence *File::getSequence() {
    return m_impl->getSequence();
}

/*!
 * Getter methods.
 * Copies of the strings are returned. Use free(...) when cleaning them up.
 */
unsigned long Sequence::length() {
    return m_length;
}

char *Sequence::getSequence() {
    if (m_sequence == NULL) {
        return NULL;
    }
    return strdup(m_sequence);
}

char *Sequence::getHeader() {
    if (m_header == NULL) {
        return NULL;
    }
    return strdup(m_header);
}

char *Sequence::getName() {
    if (m_name == NULL) {
        return NULL;
    }
    return strdup(m_name);
}

} /* namespace FASTA */
