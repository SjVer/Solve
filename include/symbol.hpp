#ifndef SYMBOL_H
#define SYMBOL_H

#include "pch"
#include "actions.hpp"

class ExprNode;

typedef struct _Target
{
	Token token;
	string name = "<invalid>";

	bool has_params = false;
	vector<string> params;

	bool invalid = true;
} Target;

typedef struct _Symbol
{
	Target target;
	int id = 0;
	ExprNode* body = nullptr;
	bool invalid = true;

	string get_ident()
	{
		string idstr = id >= 0 ? "." + to_string(id)
							   : "," + to_string(-id - 1);
		return target.name + idstr;
	}
} Symbol;


typedef struct _Environment
{
	vector<Symbol*> symbols;
	map<uint, BoundValue> bindings;
} Environment;

#endif