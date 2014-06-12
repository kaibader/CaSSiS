----------------------------------------------------------------------------
CaSSiS README
----------------------------------------------------------------------------

To report a bug or give feedback, send an email to: mail@kaibader.de
(Last edited: 2014-06-12)


----------------------------------------------------------------------------
LICENSES
----------------------------------------------------------------------------

* CaSSiS

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

* MiniPT and 3rd-party/arb

The ARB software and documentation are not in the public domain. External
programs distributed together with ARB are copyrighted by and are the property
of their respective authors unless otherwise stated. All other copyrights are
owned by Lehrstuhl fuer Mikrobiologie, TU Muenchen.

See: 'arb_license.txt' file for the complete ARB license text!

----------------------------------------------------------------------------
BUILDING CASSIS FROM THE SOURCE CODE PACKAGE:
----------------------------------------------------------------------------

CaSSiS uses the cmake build system. Please make sure to have it installed.

IN-SOURCE BUILDS ARE PREVENTED! The build will be halted.
Please make sure to build the source code in an empty directory, i.e. create
a new directory and then run cmake.

Sample session:
$  tar xvfj CaSSiS-0.5.1-src.tar.bz2
                             (e.g. extracts to /path/to/cassis)
$  mkdir /path/to/build
$  cd /path/to/build
$  cmake /path/to/cassis  (Optional build options can be added here.)
$  make


----------------------------------------------------------------------------
THE FILES SHIPPED WITH CASSIS BINARY PACKAGES:
----------------------------------------------------------------------------

Currently, only the 64 bit linux architecture is supported with a binary
release. CaSSiS was built with Ubuntu 12.04 LTS (Precise Pangolin). You can
build a 32 bit version with the available source code package.

The CaSSiS binary tar archive contains the following files:

cassis              The CaSSiS command line tool.
cassis-gui          A graphical user interface for CaSSiS. (buggy)
bgrt2graphviz       A small tool to visualize small(!) BGRT structures
bgrtmerge           A test tool to merge BGRT files
thermodynamics      A tool to test the thermodynamic functionalities
libCaSSiS.so        (symbolic link)
libCaSSiS.so.0      (symbolic link)
libCaSSiS.so.0.5.1  The CaSSiS Library v0.5.1
libminipt.so

arb_license.txt       The ARB license (for libARBDB.so and libminipt.so)
gpl.txt               The GNU General Public License v3
lgpl.txt              The GNU Lesser General Public License v3
readme.txt            Help file.
changelog.txt


----------------------------------------------------------------------------
USING CASSIS:
----------------------------------------------------------------------------

Comment: Please make sure to point 'LD_LIBRARY_PATH' to the correct directory,
if necessary. (export LD_LIBRARY_PATH="/path/to/cassis/lib)

CaSSiS usage: cassis {1pass|create|process|info} [options]

cassis 1pass
  Mandatory: -seq [... -seq] -tree
  Optional:  -all -dist -gc -idx -len -mis -og -out -rc -temp -wm
  Comment:   '1pass' uses the faster CaSSiS-LCA algorithm.

cassis create
  Mandatory: -bgrt -seq [... -seq]
  Optional:  -all -dist -gc -idx -len -mis -rc -temp -wm

cassis process
  Mandatory: -bgrt -tree|-list
  Optional:  -og -out

cassis info
  Mandatory: -bgrt

Options (alphabetical):
  -all              Evaluate all 4^len possible signatures.
                    (Not recommended, may take forever... Default: off)
  -bgrt <filename>  BGRT file path and name.
  -dist <number>    Minimal mismatch distance between a signature candidate
                    and non-targets. Must be higher than "-mis <number>".
                    (Default: 1.0 mismatches)
  -gc <min>-<max>   Only allow signatures within a defined G+C content range.
                    (Default: 0 -- 100 percent)
  -idx <name>       Defines the used search index:
                        minipt = "MiniPt Search Index" (Default)
  -len {<len>|<min>-<max>}
                    Length of the evaluated oligonucleotides. Either a
                    fixed length or a range. (Default: 18 bases)
                    Lengths must be between 10 and 25 bases.
  -list <filename>  Instead of a phylogenetic tree, a list with comma separated
                    identifiers can be used to define groups the should be
                    queried. Each line in the list defines one group.
                    The output format is set to 'sigfile'.
                    (Comment: Only available in 'cassis process'.)
  -mis <number>     Number of allowed mismatches within the target group.
                    (Default: 0.0 mismatches)
  -og <limit>       Number of outgroup hits up to which group signatures are
                    computed. (Default: 0)
  -out <format>     Defines the output format.
                        classic  = "Classic CSV format" (Default)
                        detailed = "Detailed CSV format"
                        sigfile  = "Signature file for each group/leaf"
  -rc               Drop signatures, if their reverse complement matches
                    sequences not matched by the signature itself.
                    (Default: off)
  -seq <filename>   MultiFasta file as sequence data source Multiple sequence
                    sources can be defined.
  -temp <min>-<max> Only allow signatures with a melting temperature within
                    the defined range. (Default: -273 -- 273 degree Celsius)
  -tree <filename>  Signature candidates will be computed for every defined
                    (i.e. named) node within a binary tree. Accepts a binary
                    Newick tree file as source.
  -v                Verbose output
  -wm               Enable "weighted mismatch" values. (Default: off)

Caution: Combining the "-gc" and "-temp" filters can cause unwanted side
         effects because they influence each other.

----------------------------------------------------------------------------
RESULT FILES:
----------------------------------------------------------------------------

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

Two additional output formats exist: a more detailed CSV table output can be
selected; and for each defined group and each sequence (i.e. leaf), a
separate result signature file can be generated.


----------------------------------------------------------------------------
CITATION:
----------------------------------------------------------------------------

If you find CaSSiS useful for your research or applications, please cite:

Bader, KC, Grothoff, C, Meier, H (2011). Comprehensive and relaxed search for
oligonucleotide signatures in hierarchically clustered sequence datasets.
Bioinformatics, 27, 11:1546-1554. doi: 10.1093/bioinformatics/btr161.

