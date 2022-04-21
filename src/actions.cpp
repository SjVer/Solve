#include "actions.hpp"

// =============================================
#define HANDLER(name) static double handler_##name(double* args)

HANDLER(printd) // print double
{
	cout << args[0] << endl;
	return 0;
}

#undef HANDLER
// =============================================
#define HANDLER(name, argc) { #name, argc, &handler_##name }

vector<Action> actions = {
	HANDLER(printd, 1),
};

Action* get_action(string name)
{
	for(auto it = actions.begin(); it != actions.end(); it++)
		if(it->name == name) return &(*it);
	return nullptr;
}