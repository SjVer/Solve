#include "printer.hpp"

string Printer::print(Symbol* symbol)
{
	_stream = stringstream();
	_args = new vector<ExprNode*>();

	// PRINT(symbol->get_ident() + " = ");
	symbol->body->accept(this);

	return _stream.str();
}

// =========================================
// All visit methods MUST push a value!

#define VISIT(_node) void Printer::visit(_node* node)

VISIT(AssignNode)
{
	// this kind of node should never be solved for
	THROW_INTERNAL_ERROR("during printing");
}

VISIT(BinaryNode)
{
	node->_left->accept(this);

	switch(node->_optype)
	{
		case TOKEN_EQUAL_EQUAL:		PRINT(" == "); break;
		case TOKEN_SLASH_EQUAL:		PRINT(" /= "); break;

		case TOKEN_GREATER_EQUAL:	PRINT(" >= "); break;
		case TOKEN_LESS_EQUAL:		PRINT(" <= "); break;
		case TOKEN_GREATER:			PRINT(" > "); break;
		case TOKEN_LESS:			PRINT(" < "); break;

		case TOKEN_PLUS:  			PRINT(" + "); break;
		case TOKEN_MINUS: 			PRINT(" - "); break;
		case TOKEN_STAR:  			PRINT(" * "); break;
		case TOKEN_SLASH:			PRINT(" / "); break;
		default: THROW_INTERNAL_ERROR("during printing");
	}

	node->_right->accept(this);
}

VISIT(UnaryNode)
{
	switch(node->_optype)
	{
		case TOKEN_MINUS:  	  	PRINT("-"); break;
		default: THROW_INTERNAL_ERROR("during printing");
	}
	node->_expr->accept(this);
}

VISIT(GroupingNode)
{
	PRINT("(");
	node->_expr->accept(this);
	PRINT(")");
}

VISIT(NumberNode)
{
	// just print the node's value
	PRINT(node->_value);
}

VISIT(VariableNode)
{
	PRINT("(");
	if(node->_symbol->id >= 0) node->_symbol->body->accept(this);
	else (*_args)[-node->_symbol->id - 1]->accept(this);
	PRINT(")");
}

VISIT(CallNode)
{
	// set args
	_args->clear();
	for(auto a : node->_args) _args->push_back(a);

	// visit body
	PRINT("(");
	node->_symbol->body->accept(this);
	PRINT(")");

	_args->clear();
}

#undef VISIT