#ifndef SOLVER_H
#define SOLVER_H

#include "ast.hpp"
#include "symbol.hpp"
#include "pch"

#include <cmath>

using namespace std;

class Solver: public Visitor
{
public:

	Status solve(Symbol* symbol);
	double result;
	
private:

	#define VISIT(_node) void visit(_node* node)
	#include "visits.def"
	#undef VISIT

	void push(double value);
	double pop();
	stack<double> _stack;

	vector<ExprNode*> _args;
};


#endif