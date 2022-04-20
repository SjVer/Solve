#ifndef SYMBOL_H
#define SYMBOL_H

#include "pch"

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
	uint id = 0;
	ExprNode* body = nullptr;
	bool invalid = true;

	string get_ident() { return target.name + '.' + to_string(id); }
} Symbol;

#endif