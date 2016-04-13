CaSSiS
======

CaSSiS is a fast and scalable software for computing comprehensive collections of sequence- and sequence-group-specific oligonucleotide signatures from large sets of hierarchically clustered nucleic acid sequence data. CaSSiS determines sequence-specific signatures and perfect group-covering signatures for every node within a cluster (i.e. target groups). For groups lacking a perfect common signature, it is able to find the signatures with maximal group coverage (sensitivity) within a user-defined range of non-target hits (specificity). An upper limit of tolerated mismatches within the target group, as well as the minimum number of mismatches with non-target sequences, can be predefined.

Please cite the corresponding Bioinformatics publication if you find this project useful:
[Comprehensive and relaxed search for oligonucleotide signatures in hierarchically clustered sequence datasets](http://dx.doi.org/10.1093/bioinformatics/btr161)

The former CaSSiS [project page](http://cassis.in.tum.de/) page at the TU München was shut down in the mean time. I've therefore created this 'mirror' of the source CaSSiS source code.

Building CaSSiS from Source
---------------------------

CaSSiS uses the cmake build system. Please make sure to have it installed. The build will be halted if cmake/make is run in the source directory to avoid build errors. Please make sure to build the source code in an empty directory, i.e. create a new (sub)directory and then run cmake from there.
Caution: Running 'make' in the source directory is prevented!

Source code build steps:

```
$ tar xvfj CaSSiS-0.5.2-src.tar.bz2
$ mkdir /path/to/build
$ cd /path/to/build
$ cmake /path/to/cassis-src
$ make
```

I've also prepared a video tutorial: [How To build CaSSiS from Source](https://youtu.be/1oMj-dD6GHA).
 
Usage
-----

Please make sure to point 'LD_LIBRARY_PATH' to the correct directory, if necessary.

```
(export LD_LIBRARY_PATH=/path/to/cassis/lib)
 CaSSiS usage: cassis {1pass|create|process|info} [options]
 cassis 1pass (new CaSSiS-LCA approach)
 Mandatory: -seq [... -seq] -tree
 Optional: -all -dist -gc -idx -len -mis -og -rc -temp -wm
 cassis create
 Mandatory: -bgrt -seq [... -seq]
 Optional: -all -dist -gc -idx -len -mis -rc -temp -wm
 cassis process
 Mandatory: -bgrt -tree
 Optional: -og
 cassis info
 Mandatory: -bgrt
 Options (alphabetical):
 -all Evaluate all 4^len possible signatures. (Not recommended, may take forever... Default: off)
 -bgrt BGRT file path and name.
 -dist <number> Minimal mismatch distance between a signature candidate and non-targets. (Default: 0.0 mismatches)
 -gc <min>-<max> Only allow signatures within a defined G+C content range. (Default: 0 -- 100 percent)
 -idx Defines the used search index: minipt = "MiniPt Search Index" (Default)
 -len {|-} Length of the evaluated oligonucleotides. Either a fixed length or a range. (Default: 18 bases)
 -mis <number> Number of allowed mismatches within the target group. (Default: 1.0 mismatches)
 -og <limit> Number of outgroup hits up to which group signatures are computed. (Default: 0)
 -rc Drop signatures, if their reverse complement matches sequences not matched by the signature itself. (Default: off)
 -seq MultiFasta file as sequence data source Multiple sequence sources can be defined.
 -temp <min>-<max> Only allow signatures with a melting temperature within the defined range. (Default: -273 -- 273 degree Celsius)
 -tree Signature candidates will be computed for every defined (i.e. named) node within a binary tree. Accepts a Newick tree file as source.
 -v Verbose output
 -wm Enable "weighted mismatch" values. (Default: off)
 Caution: Combining the -gc and -temp filters can cause unwanted side effects because they influence each other.
```

CaSSiS Result Files
-------------------

CaSSiS was designed to compute comprehensive signature sets based on large sequence databases. To be able to further process/parse the result files of CaSSiS, the simple but flexible CSV format was chosen. Smaller result files can be read/edited with spreadsheet software (Excel, LibreOffice, ...). Larger files should be parsed with an appropriate editor or scripts.

When directly run, CaSSiS creates two different types of result files in the CSV format in the current directory: result_array.csv and results_xxx.csv where 'xxx' is defined by the outgroup hits' range.

result_array.csv:
This file gives an overview over the results. Rows represent the range from 0 to max. outgroup hits. Columns represent the nodes of the hierarchical cluster. All information within this file is also included in the following ones.

results_xxx.csv:
This file contains the actual results of CaSSiS, separated by the number of outgroup hits 'xxx'. Within the files, the first column contains the node identifier of the hierarchical cluster. The second and third column show the number of organisms within the group/node (for leafs/organisms: 1) and the number of
actually matched organisms. The appropriate signatures follow subsequently.

License
-------

CaSSiS

CaSSiS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. CaSSiS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details. You should have received a copy of the GNU Lesser General Public License along with CaSSiS. If not, see http://www.gnu.org/licenses/

MiniPT

MiniPT is derived from the ARB PT-Server search index: The ARB software and documentation are not in the public domain. External programs distributed together with ARB are copyrighted by and are the property of their respective authors unless otherwise stated. All other copyrights are owned by the Lehrstuhl für Mikrobiologie, TU München. See the respective license.txt files in the binary/source packages for more details.
