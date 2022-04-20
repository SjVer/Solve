#ifndef AST_H
#define AST_H

#include "pch"
#include "common.hpp"
#include "scanner.hpp"

#include "symbol.hpp"

// =================================================

// forward declarate nodes
class AssignNode;
class BinaryNode;
class UnaryNode;
class GroupingNode;
class NumberNode;
class VariableNode;
class CallNode;

// visitor class
class Visitor
{
	public:
	#define VISIT(_node) virtual void visit(_node* node) = 0
	VISIT(AssignNode);
	VISIT(BinaryNode);
	VISIT(UnaryNode);
	VISIT(GroupingNode);
	VISIT(NumberNode);
	VISIT(VariableNode);
	VISIT(CallNode);
	#undef VISIT
};

// astnode class (visited by visitor)
class ExprNode
{
	public:
	ExprNode(Token token): _token(token) {}
	Token _token;
	virtual void accept(Visitor* v) = 0;
};

// =================================================

#pragma region nodes
#define ACCEPT void accept(Visitor *v) { v->visit(this); }

class AssignNode: public ExprNode
{
	public:

	AssignNode(Token token, ExprNode* target, ExprNode* expr):
		ExprNode(token), _target(target), _expr(expr) {}
	ACCEPT

	ExprNode* _target;
	ExprNode* _expr;
};

class BinaryNode: public ExprNode
{
	public:

	BinaryNode(Token token, TokenType optype, ExprNode* left, ExprNode* right):
		ExprNode(token), _optype(optype), _left(left), _right(right) {}
	ACCEPT

	TokenType _optype;
	ExprNode* _left;
	ExprNode* _right;
};

class UnaryNode: public ExprNode
{
	public:

	UnaryNode(Token token, TokenType optype, ExprNode* expr):
		ExprNode(token), _optype(optype), _expr(expr) {}
	ACCEPT

	TokenType _optype;
	ExprNode* _expr;
};

class GroupingNode: public ExprNode
{
	public:

	GroupingNode(Token token, ExprNode* expr): 
		ExprNode(token), _expr(expr) {}
	ACCEPT

	ExprNode* _expr;
};

class NumberNode: public ExprNode
{
	public:

	NumberNode(Token token, double value):
		ExprNode(token), _value(value) {}
	ACCEPT

	double _value;
};

class VariableNode: public ExprNode
{
	public:

	VariableNode(Token token, Symbol* symbol):
		ExprNode(token), _symbol(symbol) {}
	ACCEPT

	Symbol* _symbol;
};

class CallNode: public ExprNode
{
	public:

	CallNode(Token token, Symbol* symbol, vector<ExprNode*> args):
		ExprNode(token), _symbol(symbol), _args(args) {}
	ACCEPT

	Symbol* _symbol;
	vector<ExprNode*> _args;
};

#undef ACCEPT
#pragma endregion

#endif