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

	Status solve(Environment* env, Symbol* symbol);
	double result;
	
private:

	#define VISIT(_node) void visit(_node* node)
	#include "visits.def"
	#undef VISIT

	void push(double value);
	double pop();
	stack<double> _value_stack;

	Environment* _env;
	stack<vector<ExprNode*>> _args_stack;
};


#endif