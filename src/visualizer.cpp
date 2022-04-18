#include "visualizer.hpp"
#include "tools.hpp"

#define ADD_NODE(name) (_stream << \
	tools::fstr("\tnode%d [label=\"%s\"]\n", _nodecount, name), _nodecount++)

#define CONNECT_NODES(node1, node2) (_stream << \
	tools::fstr("\tnode%d -> node%d\n", node1, node2))

#define CONNECT_NODES_LABELED(node1, node2, label) (_stream << \
	tools::fstr("\tnode%d -> node%d [label=" #label ", fontsize=10, fontname=\"Courier\"]\n", node1, node2))

void ASTVisualizer::visualize(string path, AST* astree)
{
	_stream = stringstream();
	_nodecount = 1;

	_stream << HEADER;

	for(auto& node : *astree)
	{
		// node id will be _nodecount
		CONNECT_NODES(0 /* root */, _nodecount);
		node->accept(this);
	}

	_stream << FOOTER << endl;

	// write to file
	DEBUG_PRINT_MSG("Generating AST image...");
	int status = system(tools::fstr("echo '%s' | dot -Tsvg > %s", _stream.str().c_str(), path.c_str()).c_str());
	if(status) cout << _stream.str() << endl;
	// else system(tools::fstr("eog %s", path.c_str()).c_str());
	// remove(path.c_str());
}

// =========================================
// All visit methods MUST invoke ADD_NODE() at least once!
#define VISIT(_node) void ASTVisualizer::visit(_node* node)

VISIT(AssignNode)
{
	int thisnode = ADD_NODE("=");
	CONNECT_NODES(thisnode, _nodecount);
	node->_target->accept(this);

	CONNECT_NODES(thisnode, _nodecount);
	node->_expr->accept(this);
}

VISIT(BinaryNode)
{
	int thisnode = 0;
	switch(node->_optype)
	{
		case TOKEN_EQUAL_EQUAL:		thisnode = ADD_NODE("=="); break;
		case TOKEN_SLASH_EQUAL:		thisnode = ADD_NODE("/="); break;

		case TOKEN_GREATER_EQUAL:	thisnode = ADD_NODE(">="); break;
		case TOKEN_LESS_EQUAL:		thisnode = ADD_NODE("<="); break;
		case TOKEN_GREATER:			thisnode = ADD_NODE(">"); break;
		case TOKEN_LESS:			thisnode = ADD_NODE("<"); break;

		case TOKEN_PLUS:  			thisnode = ADD_NODE("+"); break;
		case TOKEN_MINUS: 			thisnode = ADD_NODE("-"); break;
		case TOKEN_STAR:  			thisnode = ADD_NODE("*"); break;
		case TOKEN_SLASH:			thisnode = ADD_NODE("/"); break;
		default: THROW_INTERNAL_ERROR("during AST visualization");
	}

	CONNECT_NODES(thisnode, _nodecount);
	node->_left->accept(this);
	CONNECT_NODES(thisnode, _nodecount);
	node->_right->accept(this);
}

VISIT(UnaryNode)
{
	int thisnode = 0;
	switch(node->_optype)
	{
		case TOKEN_MINUS:  	  	thisnode = ADD_NODE("-"); break;
		default: THROW_INTERNAL_ERROR("during AST visualization");
	}

	CONNECT_NODES(thisnode, _nodecount);
	node->_expr->accept(this);
}

VISIT(GroupingNode)
{
	int thisnode = ADD_NODE("()");
	// node id will be _nodecount
	CONNECT_NODES(thisnode, _nodecount);
	node->_expr->accept(this);
}

VISIT(NumberNode)
{
	ADD_NODE(tools::fstr("%g", node->_value).c_str());
}

VISIT(VariableNode)
{
	ADD_NODE(node->_ident.c_str());
}

VISIT(CallNode)
{
	int thisnode = ADD_NODE(tools::fstr("%s ()", node->_ident.c_str()).c_str());

	for(auto& subnode : node->_args)
	{
		// node id will be _nodecount
		CONNECT_NODES(thisnode, _nodecount);
		subnode->accept(this);
	}
}

#undef ADD_NODE
#undef CONNECT_NODES
#undef VISIT