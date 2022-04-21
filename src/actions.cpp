#include "actions.hpp"
#include "symbol.hpp"
#include "error.hpp"
#include "tools.hpp"

// ==================================================

string BoundValue::to_string(bool debug)
{
	if(debug) switch(type)
	{
		case NUMBER: return ::to_string(as.num);
		case STRING: return  '"' + string(as.str) + '"';
		default: return "<invalid bound value>";
	}
	else switch(type)
	{
		case NUMBER: return tools::fstr("%g", as.num);
		case STRING: return tools::escstr(as.str);
		default: return "<invalid bound value>";
	}
}

// ==================================================

static ErrorDispatcher err_dispatcher = ErrorDispatcher();
#define ERR_PROMPT "Runtime Error"
#define FMT_ERROR(fmt, ...) err_dispatcher.error_at_token(tok, ERR_PROMPT, tools::fstr(fmt, __VA_ARGS__).c_str())

static BoundValue get_bound_value(Token* tok, Environment* env, double addr)
{
	if(addr != (uint)addr) // make sure addr is uint
	{
		FMT_ERROR("Bind address %g is not an unsigned integer.", addr);
		ABORT(STATUS_SOLVE_ERROR);
	}
	else if(env->bindings.find(addr) == env->bindings.end()) // check if addr bound
	{
		FMT_ERROR("No value bound to address 0x%x (%u).", (uint)addr, (uint)addr);
		ABORT(STATUS_SOLVE_ERROR);
	}

	return env->bindings[addr];
}

// ==================================================

#pragma region handlers
#define HANDLER(name) static double handler_##name(Token* tok, Environment* env, double* args)
#define GET_BOUND_VALUE(addr) get_bound_value(tok, env, addr)

HANDLER(print) // print double
{
	cout << args[0] << endl;
	return 0;
}

HANDLER(printb) // print string
{
	cout << GET_BOUND_VALUE(args[0]).to_string(false) << endl;
	return 0;
}

HANDLER(int) // cast to int
{
	//
	return (int)args[0];
}

HANDLER(get) // gets a double
{
	BoundValue val = GET_BOUND_VALUE(args[0]);
	if(val.type != BoundValue::NUMBER)
		FMT_ERROR("Cannot use non-numeric bound value %s.", val.to_string(true).c_str());
	return val.as.num;
}

#undef HANDLER
#pragma endregion

// built-in actions
#define HANDLER(name, argc) { #name, argc, &handler_##name }
vector<Action> actions = {
	HANDLER(print, 1),
	HANDLER(printb, 1),
	HANDLER(int, 1),
	HANDLER(get, 1),
};
#undef HANDLER

Action* get_action(string name)
{
	for(auto it = actions.begin(); it != actions.end(); it++)
		if(it->name == name) return &(*it);
	return nullptr;
}
