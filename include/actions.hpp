#ifndef ACTIONS_H
#define ACTIONS_H

#include "common.hpp"
#include "scanner.hpp"
#include "pch"

typedef struct _Action
{
	string name;
	uint arity;
	double (*handler)(Token*, struct _Environment*, double*);
} Action;

typedef struct _BoundValue
{
	union _as
	{
		double num;
		char* str;
	} as;

	enum _type
	{
		NUMBER,
		STRING,
	} type;

	string to_string(bool debug);
} BoundValue;

extern vector<Action> actions;

Action* get_action(string name);

#endif