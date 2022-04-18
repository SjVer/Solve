#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "ast.hpp"
#include "pch"

using namespace std;

class ASTVisualizer: public Visitor
{
	public:
	void visualize(string path, AST* astree);
	
	#define VISIT(_node) void visit(_node* node)
	#include "visits.def"
	#undef VISIT

	private:
	stringstream _stream;
	int _nodecount;
};

#define HEADER "digraph astgraph {\n\
	node [shape=rect, fontsize=12, fontname=\"Courier\", height=.1];\n\
	ranksep=.4;\n\
	edge [arrowsize=.5, arrowhead=\"none\"]\n\
	rankdir=\"UD\"\n\
	node0 [label=\"Program\"]\n\
	\n\
"
#define FOOTER "}"

#endif