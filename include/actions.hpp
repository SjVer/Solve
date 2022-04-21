#ifndef ACTIONS_H
#define ACTIONS_H

#include "common.hpp"
#include "pch"

typedef struct _Action
{
	string name;
	uint arity;
	double (*handler)(double*);
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

	string to_string()
	{
		switch(type)
		{
		case NUMBER: return ::to_string(as.num);
		case STRING: return string(as.str);
		default: return "<invalid bound value>";
		}
	}
} BoundValue;

extern vector<Action> actions;

Action* get_action(string name);

#endif