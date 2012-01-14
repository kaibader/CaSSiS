/*!
 * MultiFASTA formatted file reader
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
 *
 * Copyright (C) 2010,2011
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

#ifndef FASTA_H_
#define FASTA_H_

namespace FASTA {

/*!
 * Some predefinitions...
 */
class FileImpl;
class Sequence;

/*!
 * Sequence data type (IUPAC alphabet).
 */
enum Type {
    UNDEF = -1, RNA = 0, DNA = 1, AMINO = 2
};

/*!
 * MultiFASTA file class
 */
class File {
public:
    /*!
     * Constructor, including parameters.
     *
     * \param filename FASTA file name
     * \param Used alphabet: "DNA,RNA,AMINO"-IUPAC code. Needed for input-filtering.
     */
    File(const char *filename, Type type);

    /*!
     * Destructor.
     */
    ~File();

    /*!
     * File open indicator.
     *
     * \return true, if a FASTA file was successfully opened.
     */
    bool isOpen();

    /*!
     * End of file indicator.
     *
     * \return true, if end of file is reached or no file is opened.
     */
    bool atEOF();

    /*!
     * Close file.
     */
    void close();

    /*!
     * Fetch the next sequence
     *
     * \return Sequence object (pointer), if successful. Otherwise NULL.
     */
    Sequence *getSequence();
private:
    File(const File&); // No copy constructor
    File& operator=(File); // No copy assignment operator
    FileImpl *m_impl; // Implementation class of the FastaFile class
};

/*!
 * A sequence entry from a FASTA formatted file. Read only!
 */
class Sequence {
public:
    /*!
     * Constructor.
     */
    Sequence();

    /*!
     * Destructor.
     */
    virtual ~Sequence();

    /*!
     * Copy constructor.
     */
    Sequence(const Sequence &value);

    /*!
     * Copy assignment operator.
     */
    Sequence& operator=(Sequence value);

    /*!
     * Getter methods.
     * Copies of the strings are returned. Use free(...) when cleaning them up.
     */
    unsigned long length();
    char *getSequence();
    char *getHeader();
    char *getName();
private:
    /*!
     * This is the complete header of a sequence (all chars after the '>').
     * String is terminated with '0'.
     */
    char *m_header;

    /*!
     * This is the first word in the header.
     * String is terminated with '0'.
     * Caution: No check for uniqueness is done.
     */
    char *m_name;

    /*!
     * This is the sequence data, filtered according to the sequence data type.
     * Spaces and newlines are stripped, string is terminated with '0'.
     */
    char *m_sequence;

    /*!
     * Sequence length (number of chars within 'sequence').
     */
    unsigned long m_length;

    /*!
     * Defines the sequence type.
     */
    Type m_type;

    /*!
     * Allow FastaFile and FastaFileImpl access to private members...
     */
    friend class File;
    friend class FileImpl;
};

} /* namespace FASTA */
#endif /* FASTA_H_ */
