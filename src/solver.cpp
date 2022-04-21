#include "solver.hpp"

Status Solver::solve(Environment* env, Symbol* symbol)
{
	// reset result real quick
	result = nan("<no result>");
	_args = vector<ExprNode*>();
	_env = env;

	symbol->body->accept(this);
	result = pop();
	if(isnan(result)) result = nan("<NaN>");

	return STATUS_SUCCESS;
}

void Solver::push(double value)
{
	// just push the value
	_stack.push(value);
}

double Solver::pop()
{
	ASSERT_OR_THROW_INTERNAL_ERROR(!_stack.empty(), "during solving");
	double value = _stack.top();
	_stack.pop();
	return value;
}

// =========================================
// All visit methods MUST push a value!

#define VISIT(_node) void Solver::visit(_node* node)

VISIT(AssignNode)
{
	// this kind of node should never be solved for
	THROW_INTERNAL_ERROR("during solving");
}

VISIT(BinaryNode)
{
	node->_left->accept(this);
	double lhs = pop();
	node->_right->accept(this);
	double rhs = pop();

	switch(node->_optype)
	{
		case TOKEN_EQUAL_EQUAL:		push(lhs == rhs); break;
		case TOKEN_SLASH_EQUAL:		push(lhs /= rhs); break;

		case TOKEN_GREATER_EQUAL:	push(lhs >= rhs); break;
		case TOKEN_LESS_EQUAL:		push(lhs <= rhs); break;
		case TOKEN_GREATER:			push(lhs > rhs); break;
		case TOKEN_LESS:			push(lhs < rhs); break;

		case TOKEN_PLUS:  			push(lhs + rhs); break;
		case TOKEN_MINUS: 			push(lhs - rhs); break;
		case TOKEN_STAR:  			push(lhs * rhs); break;
		case TOKEN_SLASH:			push(lhs / rhs); break;
		default: THROW_INTERNAL_ERROR("during solving");
	}
}

VISIT(UnaryNode)
{
	node->_expr->accept(this);
	double val = pop();

	switch(node->_optype)
	{
		case TOKEN_MINUS:  	  	push(- val); break;
		default: THROW_INTERNAL_ERROR("during solving");
	}
}

VISIT(GroupingNode)
{
	// just accept expr inside
	node->_expr->accept(this);
}

VISIT(NumberNode)
{
	// just push the node's value
	push(node->_value);
}

VISIT(VariableNode)
{
	if(node->_symbol->id >= 0) node->_symbol->body->accept(this);
	else _args[-node->_symbol->id - 1]->accept(this);
}

VISIT(CallNode)
{
	// set args
	_args.clear();
	for(auto a : node->_args) _args.push_back(a);

	// visit body
	node->_symbol->body->accept(this);

	_args.clear();
}

VISIT(ActionNode)
{
	double* args = new double[node->_args.size()];
	for(int i = 0; i < node->_args.size(); i++)
	{
		node->_args[i]->accept(this);
		args[i] = pop();
	}

	push(node->_action->handler(&node->_token, _env, args));
}

#undef VISIT