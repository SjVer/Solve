#include "parser.hpp"
#include "tools.hpp"

// ====================== errors =======================

void Parser::error_at(Token *token, string message)
{
	if(_panic_mode) return;

	_had_error = true;
	_panic_mode = true;

	_error_dispatcher.error_at_token(token, "Syntax Error", message.c_str());

	// print token
	if(token->type != TOKEN_ERROR)
	{
		cerr << endl;
		_error_dispatcher.print_token_marked(token, COLOR_RED);
	}
}

// displays an error at the previous token with the given message
void Parser::error(string message)
{
	//
	error_at(&_previous, message);
}

// displays an error at the current token with the given message
void Parser::error_at_current(string message)
{
	//
	error_at(&_current, message);
}

// displays note of declaration of token and line of token
void Parser::note_declaration(string type, string name, Token* token)
{
	CCP msg = strdup((type + " '" + name + "' declared here:").c_str());

	_error_dispatcher.note_at_token(token, msg);
	cerr << endl;
	_error_dispatcher.print_token_marked(token, COLOR_GREEN);
}

// ====================== scanner ======================

// advances to the next token
void Parser::advance(bool skip_terminator)
{
	_previous = _current;

	for (;;)
	{
		_current = _scanner.scanToken();

		if(_current.type == TOKEN_ERROR)
			error_at_current(_current.start);

		else if(skip_terminator && _current.type == TOKEN_NEWLINE);

		else break;
	}
}

// checks if the current token is of the given type
bool Parser::check(TokenType type)
{
	//
	return _current.type == type;
}

// consume the next token if it is of the correct type,
// otherwise throw an error with the given message
bool Parser::consume(TokenType type, string message)
{
	if (_current.type == type)
	{
		advance();
		return true;
	}
	error_at_current(message);
	return false;
}

bool Parser::consume_terminator()
{
	if(match(TOKEN_NEWLINE) || is_at_end())
	{
		while(match(TOKEN_NEWLINE));
		return true;
	}
	return false;
}

// returns true and advances if the current token is of the given type
bool Parser::match(TokenType type)
{
	if (!check(type))
		return false;
	advance();
	return true;
}

// checks if EOF reached
bool Parser::is_at_end()
{
	//
	return _current.type == TOKEN_EOF;
}

// ======================= state =======================

void Parser::set_target(string name, Target target)
{
	//
	_targets[name] = target;
}

Parser::Target Parser::get_target(string name)
{
	if(_targets.find(name) != _targets.end()) return _targets.at(name);
	else return {.invalid = true};
}

bool Parser::check_target(string name)
{
	return !get_target(name).invalid;
}

void Parser::synchronize()
{
	_panic_mode = false;
	
	while(!is_at_end() && _current.type != TOKEN_NEWLINE) advance();
}

// ===================== expressions ====================

void Parser::assignment()
{
	Target target = consume_target();
	if(target.invalid && !consume(TOKEN_EQUAL, "Expected assignment.")) return;
	
	ExprNode* body = expression();


	consume_terminator();
}

ExprNode* Parser::expression()
{
	return equality();
}

ExprNode* Parser::equality()
{
	ExprNode* expr = comparison();

	while(match(TOKEN_SLASH_EQUAL) || match(TOKEN_EQUAL_EQUAL))
	{
		Token tok = _previous;
		ExprNode* right = comparison();
		expr = new BinaryNode(tok, tok.type, expr, right);
	}

	return expr;
}

ExprNode* Parser::comparison()
{
	ExprNode* expr = term();

	while(match(TOKEN_GREATER) || match(TOKEN_GREATER_EQUAL)
	   || match(TOKEN_LESS)    || match(TOKEN_LESS_EQUAL))
	{
		Token tok = _previous;
		ExprNode* right = term();
		expr = new BinaryNode(tok, tok.type, expr, right);
	}

	return expr;
}

ExprNode* Parser::term()
{
	ExprNode* expr = factor();
	
	while(match(TOKEN_PLUS) || match(TOKEN_MINUS))
	{
		Token tok = _previous;
		ExprNode* right = factor();
		expr = new BinaryNode(tok, tok.type, expr, right);
	}

	return expr;
}

ExprNode* Parser::factor()
{
	ExprNode* expr = unary();
	
	while(match(TOKEN_STAR) || match(TOKEN_SLASH))
	{
		Token tok = _previous;
		ExprNode* right = unary();
		expr = new BinaryNode(tok, tok.type, expr, right);
	}

	return expr;
}

ExprNode* Parser::unary()
{
 	if(match(TOKEN_MINUS))
	{
		Token tok = _previous;
		ExprNode* expr = unary();
		return new UnaryNode(tok, tok.type, expr);
	}

	return primary();
}

ExprNode* Parser::primary()
{
	// literals
	if(match(TOKEN_INTEGER) || match(TOKEN_FLOAT)) return literal();

	// grouping
	else if(match(TOKEN_LEFT_PAREN))
	{
		Token tok = _previous;
		ExprNode* expr = expression();
		CONSUME_OR_RET_NULL(TOKEN_RIGHT_PAREN, "Expected ')' after parenthesized expression.");
		return new GroupingNode(tok, expr);
	}

	// variables
	else if(match(TOKEN_IDENTIFIER) && !check(TOKEN_LEFT_PAREN)) return variable();

	// calls
	else if (match(TOKEN_IDENTIFIER) && check(TOKEN_LEFT_PAREN)) return call();

	error_at_current("Expected expression.");
	return nullptr;
}

// ===================== primaries =====================

NumberNode* Parser::literal()
{
	switch(_previous.type)
	{
		case TOKEN_INTEGER:
		{
			int base = 0;
			string tok = PREV_TOKEN_STR;
			
				 if(tok.size() > 2 && tolower(tok[1]) == 'b') base = 2;
			else if(tok.size() > 2 && tolower(tok[1]) == 'c') base = 8;
			else if(tok.size() > 2 && tolower(tok[1]) == 'x') base = 16;
			else base = 10;

			long intval = strtol(tok.c_str() + (base != 10 ? 2 : 0), NULL, base);
			return new NumberNode(_previous, intval);
		}
		case TOKEN_FLOAT:
		{
			string tok = PREV_TOKEN_STR;
			double doubleval = strtod(tok.c_str(), NULL);
			return new NumberNode(_previous, doubleval);
		}
		default: THROW_INTERNAL_ERROR("during parsing");
	}
	return nullptr;
}

VariableNode* Parser::variable()
{
	return new VariableNode(_previous, PREV_TOKEN_STR);
}

CallNode* Parser::call()
{
	Token tok = _previous;
	string name = PREV_TOKEN_STR;

	CONSUME_OR_RET_NULL(TOKEN_LEFT_PAREN, "Expected '(' after identifier.");

	// if(!check_function(name)) error_at(&tok, "Function does not exist in current scope.");

	vector<ExprNode*> args;
	// FuncProperties funcprops = get_function_props(name);
	// int paramscount = funcprops.params.size();
	
	if(!check(TOKEN_RIGHT_PAREN)) do args.push_back(expression()); while(match(TOKEN_COMMA));

	// if(args.size() != paramscount)
	// {
	// 	HOLD_PANIC();
	// 	error_at(&tok, tools::fstr("Expected %s%d argument%s, but %d were given.",
	// 		funcprops.variadic ? "at least " : "", paramscount, paramscount == 1 ? "" : "s", args.size()));
	// 	if(!PANIC_HELD) note_declaration("Function", name, &funcprops.token);
	// 	return nullptr;
	// }

	CONSUME_OR_RET_NULL(TOKEN_RIGHT_PAREN, "Expected ')' after arguments.");

	// return funcprops.invalid ? nullptr : new CallNode(tok, name, args, funcprops.ret_type, lexparams, paramscount);
	return new CallNode(tok, name, args);
}

// ======================= misc. =======================

Status Parser::parse(string infile, CCP source, AST* astree)
{
	// print_tokens_from_src(source);
	_astree = astree;

	// set members
	_scanner = Scanner(&infile, source);

	_had_error = false;
	_panic_mode = false;
	_error_dispatcher = ErrorDispatcher();

	_main_file = infile;

	advance();
	while (!is_at_end())
	{
		assignment();
		if(_panic_mode) synchronize();
	}

	DEBUG_PRINT_MSG("Parsing complete!");
	return _had_error ? STATUS_PARSE_ERROR : STATUS_SUCCESS;
}