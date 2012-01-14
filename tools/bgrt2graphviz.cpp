/*!
 * BGRT -> Graphviz (tree converter tool)
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) tools.
 *
 * Copyright (C) 2012
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

#include <cassis/bgrt.h>
#include <cassis/io.h>

#include <iostream>
#include <fstream>
#include <cstring>

void traverseBGRT(unsigned long &nodecounter, unsigned long parentcounter,
		const BgrTreeNode *node, const NameMap &map, std::ostream &stream) {
	if (!node)
		return;
	++nodecounter;

	stream << "n" << nodecounter << " [shape=\"circle\" label=\"\"];\n";

	//stream << "n" << nodecounter << " [label=\"" << node->species->size()
	//				<< "\"];\n";

	//stream << "n" << nodecounter << " [label=\"";
	//if (node->species->size() > 0)
	//	stream << node->species->val(0);
	//for (unsigned int i = 1; i < node->species->size(); ++i)
	//	stream << "," << node->species->val(i);
	//stream << "\"];\n";

	//stream << "n" << nodecounter << " [label=\"";
	//if (node->species->size() > 0)
	//	stream << map.name(node->species->val(0));
	//for (unsigned int i = 1; i < node->species->size(); ++i)
	//	stream << "," << map.name(node->species->val(i));
	//stream << "\"];\n";

	// if (parentcounter > 0)
	stream << "n" << parentcounter << " -> n" << nodecounter << ";\n";

	// Traverse child nodes...
	BgrTreeNode *children = node->children;
	unsigned long mycounter = nodecounter;
	while (children) {
		traverseBGRT(nodecounter, mycounter, children, map, stream);
		children = children->next;
	}
}

int main(int argc, char **argv) {
	// Check if the right number of arguments is given.
	if (argc != 3 || !strcmp(argv[1], "/?") || !strcmp(argv[1], "-h")
			|| !strcmp(argv[1], "--help")) {
		std::cout << "Usage: bgrt2Graphviz <bgrt-file> <Graphviz-file>\n"
				"This tool converts a BGRT into a Graphviz (.gv) file.\n";
		return 0;
	}

	// Create a NameMap and open the BGRT file.
	NameMap map;
	BgrTree *bgr_tree = readBGRTFile(&map, argv[1]);
	if (!bgr_tree) {
		std::cerr << "Error: unable to open/read the BGRT file.\n";
		return 1;
	}

	// Open the Graphviz file (into which the BGRT will be converted).
	std::fstream file;
	file.open(argv[2], std::ios::in | std::ios::out | std::ios::trunc);
	if (file.bad()) {
		// Something went wrong...
		std::cerr << "Error: unable to open/create the Graphviz output file.\n";
		return 1;
	}

	// Create a Graphviz header and traverse the BGRT.
	unsigned long nodecounter = 0;
	file << "/* Created with bgrt2Graphviz\n"
			"   BGRT comment: " << bgr_tree->comment
			<< "\n*/\ndigraph G {\nn0 [label=\"root\"];\n";
	for (unsigned long i = 0; i < bgr_tree->num_species; ++i)
		traverseBGRT(nodecounter, 0, bgr_tree->nodes[i], map, file);
	file << "}\n";

	// Write a short help message.
	std::cout << "Successfully created Graphviz file. "
			"I recommend following command for parsing:\n"
			"twopi -Tsvg " << argv[2] << " -o " << argv[2] << ".svg\n";

	// Clean-up...
	BgrTree_destroy(bgr_tree);
	return 0;
}
