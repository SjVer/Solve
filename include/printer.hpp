#ifndef PRINTER_H
#define PRINTER_H

#include "ast.hpp"
#include "symbol.hpp"
#include "pch"

using namespace std;

class Printer: public Visitor
{
public:

	string print(Symbol* symbol);

private:

	#define VISIT(_node) void visit(_node* node)
	#include "visits.def"
	#undef VISIT

	#define PRINT(what) { _stream << what; }
	stringstream _stream;

	stack<vector<ExprNode*>> _args_stack;
};


#endif