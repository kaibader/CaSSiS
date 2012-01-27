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

#ifndef MINIPT_IO_H
#define MINIPT_IO_H

namespace minipt {

int compress_data(char *probestring);
void cleanup_probe_compress_sequence();
int probe_compress_sequence(char *seq, int seqsize);

} /* namespace minipt */

#endif /* MINIPT_IO_H */
