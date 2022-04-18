#ifndef PARSER_H
#define PARSER_H

#include "scanner.hpp"
#include "ast.hpp"
#include "error.hpp"
#include "common.hpp"
#include "pch"

using namespace std;

// ==== ============= ====

class Parser
{
public:
	Status parse(string infile, CCP source, AST* astree);

private:

	// types

	typedef struct
	{
		Token token;
		vector<string> args;
		ExprNode* body = nullptr;

		bool invalid = true;
	} Target;

	// methods

	void error_at(Token *token, string message);
	void error(string message);
	void error_at_current(string message);
	void note_declaration(string type, string name, Token* token);

	void advance(bool skip_terminator = true);
	bool check(TokenType type);
	bool consume(TokenType type, string message);
	bool consume_terminator();
	Target consume_target();
	bool match(TokenType type);
	bool is_at_end();

	#define CONSUME_OR_RET_NULL(type, msg) if(!consume(type, msg)) return nullptr;

	void set_target(string name, Target target);
	Target get_target(string name);
	bool check_target(string name);
	void synchronize();

	void assignment();
	ExprNode* expression();
		ExprNode* equality();
		ExprNode* comparison();
		ExprNode* term();
		ExprNode* factor();
		ExprNode* unary();
		ExprNode* primary();
			NumberNode* literal();
			VariableNode* variable();
			CallNode* call();

	// members

	Scanner _scanner;
	Token _current;
	Token _previous;

	map<string, Target> _targets;

	bool _had_error;
	bool _panic_mode;
	ErrorDispatcher _error_dispatcher;

	#define HOLD_PANIC() bool _old_panic_mode_from_macro_hold_panic = _panic_mode
	#define PANIC_HELD (_old_panic_mode_from_macro_hold_panic)

	string _main_file;

	#define PREV_TOKEN_STR std::string(_previous.start, _previous.length)
};

#endif