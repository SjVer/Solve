#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "ast.hpp"
#include "symbol.hpp"
#include "pch"

using namespace std;

class ASTVisualizer: public Visitor
{
public:

	void init();
	void finalize();
	void visualize(string path, Symbol* symbol);
	
private:

	#define VISIT(_node) void visit(_node* node)
	#include "visits.def"
	#undef VISIT

	stringstream _stream;
	string _path;
	int _nodecount;
};

#define HEADER "digraph astgraph {\n\
	node [shape=rect, fontsize=12, fontname=\"Courier\", height=.1];\n\
	ranksep=.4;\n\
	edge [arrowsize=.5, arrowhead=\"none\"]\n\
	rankdir=\"UD\"\n\
	\n\
"
#define FOOTER "}"

#endif