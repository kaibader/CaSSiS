----------------------------------------------------------------------------
CaSSiS README
----------------------------------------------------------------------------

To report a bug or give feedback, send an email to: mail@kaibader.de
(Last edited: 2011-12-22)


----------------------------------------------------------------------------
LICENSES

CaSSiS
------

CaSSiS is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

CaSSiS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.

MiniPT
------


3rd-party/arb
-------------



----------------------------------------------------------------------------
THE FILES SHIPPED WITH CASSIS:

There are two versions of archives, one for 32 bit Linux architectures and one
for 64 bit architectures. CaSSiS was tested with Ubuntu 10.04 LTS (Lucid Lynx)
and Ubuntu 11.04 (Oneiric Ocelot). Other newer Linux distributions (2010 and
younger) should also work.

The CaSSiS tar archive contains the following files:

./0.3.2_64-BIT         Marker; 32/64 bit version of CaSSiS
./batch_job.sh         This is a batch script to run multiple CaSSiS jobs
./cassis               The CaSSiS program
./cdcassis             A Qt4 user interface version of CaSSiS
./ChangeLog_0.3.2.txt  Changes between the versions
./README.txt           This README
./test.batch           A sample batch job (run with 'batch_job.sh')

./lib/arb_LICENSE.txt  The ARB DB + PT-Server software license
./lib/libARBDB.so      The ARB database library
./lib/libbgrt.so       The BGRT library
./lib/libCORE.so       The ARB core functionalities library
./lib/libptserver.so   The ARB PT-Server library

./test_db/100.arb      Test databases (100-500 sequences + trees)
./test_db/200.arb
./test_db/500.arb

CaSSiS depends on the ARB PT-Server for the extraction of signature candidates
and on the ARBDB library for handling ARB database files. The CaSSiS archive
contains appropriate versions of these two files. They are based on the
development version of the ARB software project (subversion repository
revision #6981). CaSSiS is linked against this library version and may not
work when used in combination with other ARB software project revisions.


----------------------------------------------------------------------------
USING THE BATCH_JOB SCRIPT:

batch_job <batch_file>
<batch_file> = File, containing a list of batch jobs (i.e. parameter lists).

NOTE: When running CaSSiS indirectly with the batch job script, there should
be no need to configurate any environment variables.

A batch job script has a simple layout, a list of parameters. Each line
represents a single job. Comments are preceeded by a hash (#) symbol.

BATCH FILE::
</path/to/result_dir> <bgrt file> </path/to/arb_db> <arb_tree> \\
<oligo_length> <outgroup_limit> <ingroup_mismatches> <mismatch_distance>


----------------------------------------------------------------------------
USING CASSIS:

There is no need to configure the ARB environment variable -- CaSSiS is
shipped with its own libraries. Please make sure to point 'LD_LIBRARY_PATH'
to the correct directory when using CaSSiS directly:
$ export LD_LIBRARY_PATH="/path/to/cassis/lib"


CaSSiS usage: cassis {create|traverse|info} <bgrt-file> [options]

cassis create <bgrt-file> [options]

Creates a BGRT based on sequence data and a search index, and stores it under
the specified file name.

Options:
  -index {pts}      Defines the used search index.
                    pts="ARB Pt-Server"
  -arb <ARB-file>   ARB database file with sequence data.
  -length {<len>|<min>-<max>}  Either a fixed length or length range for the
                    signature candidates in the BGRT. (Default: 18)
  -mismatches <mm>  Number of allowed mismatches within the target group.
                    (Default: 1)
  -distance <dist>  Minimal mismatch distance between a signature candidate
                    and non-targets. (Default: 0)
  -weighted         Enable the use of weighted mismatches. (Default: off)
  -reverse-check    Drop signatures, if their reverse complement matches
                    sequences not matched by the signature itself.
                    (Default: off)
  -gc <min>-<max>   A G+C content filter. Only signatures with a G+C content
                    within the defined percentage.
  -temp <min>-<max> Only allow signatures with a melting temperature within
                    the defined temperature range.

cassis traverse <bgrt-file> [options]

Based on the BGRT file, CaSSiS computes signature candidates for every node
within a phylogenetic tree or a list of species.

Options:
  -arb-tree <ARB-file> <tree-name>  Load an ARB database file and select
                    the defined tree for the traversal.
  -tree <filename>  A Newick tree file is used for the traversal.
  -list <filename>  A comma separated list of species identifiers is used for
                    the traversal. It will be interpreted as a single group.
  -outgroup <limit> Number of outgroup hits up to which group signatures are
                    computed. (Default: 0)
  -gc <min>-<max>   A G+C content filter. Only signatures with a G+C content
                    within the defined percentage.
  -temp <min>-<max> Only allow signatures with a melting temperature within
                    the defined temperature range.

Note: Combining the -gc and -temp filters can cause unwanted side effects
because they influence each other.

cassis info <bgrt-file>

Show information about the BGRT file.


----------------------------------------------------------------------------
RESULT FILES:

CaSSiS was designed to compute comprehensive signature sets based on large
sequence databases. To be able to further process/parse the result files of
CaSSiS, the simple but flexible CSV format was chosen. Smaller files can be
read/edited with spreadsheet software (Excel, OpenOffice,...). Larger files
should be parsed with an appropriate editor.

When directly run, CaSSiS creates two different types of result files in the
CSV format in the current directory: 'result_array.csv' and 'results_xxx.csv'
where 'xxx' is defined by the outgroup hits range.

result_array.csv:
This file gives an overview of the results. The rows represent the range from
0 to max. outgroup hits. Columns represent the nodes of the hierarchical
cluster. All information within this file is also included in the following
ones.

results_xxx.csv:
This file contains the actual results of CaSSiS, seperated by their number of
outgroup hits 'xxx'. The first column contains the node identifier within the
hierarchical cluster. The second and third column show the number of organisms
within the group/node (for leafs/organisms: 1) and the number of acutally hit
organisms by the signatures for this node. The appropriate signatures follow
subsequently.


----------------------------------------------------------------------------
CITATION:

If you find CaSSiS useful for your research or applications, please cite:

Bader, KC, Grothoff, C, Meier, H (2011). Comprehensive and relaxed search for
oligonucleotide signatures in hierarchically clustered sequence datasets.
Bioinformatics, 27, 11:1546-1554. doi: 10.1093/bioinformatics/btr161.
