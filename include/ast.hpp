#ifndef AST_H
#define AST_H

#include "common.hpp"
#include "pch"
#include "scanner.hpp"

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
class ASTNode
{
	public:
	ASTNode(Token token): _token(token) {}
	Token _token;
	virtual void accept(Visitor* v) = 0;
};

// abstract syntax tree
typedef vector<ASTNode*> AST;

// =================================================

#pragma region nodes
#define ACCEPT void accept(Visitor *v) { v->visit(this); }

class AssignNode: public ASTNode
{
	public:

	AssignNode(Token token, ASTNode* target, ASTNode* expr):
		ASTNode(token), _target(target), _expr(expr) {}
	ACCEPT

	ASTNode* _target;
	ASTNode* _expr;
};

class BinaryNode: public ASTNode
{
	public:

	BinaryNode(Token token, TokenType optype, ASTNode* left, ASTNode* right):
		ASTNode(token), _optype(optype), _left(left), _right(right) {}
	ACCEPT

	TokenType _optype;
	ASTNode* _left;
	ASTNode* _right;
};

class UnaryNode: public ASTNode
{
	public:

	UnaryNode(Token token, TokenType optype, ASTNode* expr):
		ASTNode(token), _optype(optype), _expr(expr) {}
	ACCEPT

	TokenType _optype;
	ASTNode* _expr;
};

class GroupingNode: public ASTNode
{
	public:

	GroupingNode(Token token, ASTNode* expr): 
		ASTNode(token), _expr(expr) {}
	ACCEPT

	ASTNode* _expr;
};

class NumberNode: public ASTNode
{
	public:

	NumberNode(Token token, double value):
		ASTNode(token), _value(value) {}
	ACCEPT

	double _value;
};

class VariableNode: public ASTNode
{
	public:

	VariableNode(Token token, string ident):
		ASTNode(token), _ident(ident) {}
	ACCEPT

	string _ident;
};

class CallNode: public ASTNode
{
	public:

	CallNode(Token token, string ident, vector<ASTNode*> args):
		ASTNode(token), _ident(ident), _args(args) {}
	ACCEPT

	string _ident;
	vector<ASTNode*> _args;
};

#undef ACCEPT
#pragma endregion

#endif