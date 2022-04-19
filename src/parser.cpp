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

// bool Parser::consume_terminator()
// {
// 	if(match(TOKEN_NEWLINE) || is_at_end())
// 	{
// 		while(match(TOKEN_NEWLINE));
// 		return true;
// 	}
// 	return false;
// }

Target Parser::consume_target()
{
	Target target = Target();

	if(!consume(TOKEN_IDENTIFIER, "Expected symbol name.")) return target;
	target.name = PREV_TOKEN_STR;
	target.token = _previous;

	if(match(TOKEN_LEFT_PAREN)) // function
	{
		target.has_params = true;
		target.params = vector<string>();

		if(!check(TOKEN_RIGHT_PAREN)) do
		{
			consume(TOKEN_IDENTIFIER, "Expected parameter.");
			// target.params.push_back(pair<string, Token>(PREV_TOKEN_STR, _previous));
			target.params.push_back(PREV_TOKEN_STR);

		} while(match(TOKEN_COMMA));

		consume(TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");
	}
	
	return target;
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

void Parser::set_symbol(Symbol symbol)
{
	// find last symbol with same name and get its ID
	for(auto s = _symbols.rbegin(); s != _symbols.rend(); s++)
		if(s->target.name == symbol.target.name) { symbol.id = s->id + 1; break; }

	_symbols.push_back(symbol);
	_current_scope.symbols[symbol.target.name] = &_symbols.back();
	// _current_scope.symbols[symbol.target.name] = symbol;

	#ifdef DEBUG
	string msg = "set symbol '" + symbol.get_ident() + '\'';
	if(symbol.target.has_params)
	{
		msg += " (";
		for(auto p: symbol.target.params) msg += p + (p != symbol.target.params.back() ? ", " : "");
		msg += ')';
	}
	DEBUG_PRINT_F_MSG("%s", msg.c_str());
	#endif
}

Symbol* Parser::get_symbol(string name)
{
	if(_current_scope.symbols.find(name) != _current_scope.symbols.end())
		return _current_scope.symbols.at(name);

	for (auto s = _scope_stack.rbegin(); s != _scope_stack.rend(); s++)
		if (s->symbols.find(name) != s->symbols.end())
			return s->symbols.at(name);

	return nullptr;
}

bool Parser::check_symbol(string name)
{
	return get_symbol(name) != nullptr;
	// return !get_symbol(name).invalid;
}

void Parser::scope_up()
{
	DEBUG_PRINT_MSG("scope up");
	_scope_stack.push_back(_current_scope);
	_current_scope = Scope{map<string, Symbol*>()};
}

void Parser::scope_down()
{
	DEBUG_PRINT_MSG("scope down");
	for(auto s : _current_scope.symbols) DEBUG_PRINT_F_MSG("    dump %s -> %s", 
		s.first.c_str(), s.second->get_ident().c_str());

	_current_scope = _scope_stack[_scope_stack.size() - 1];
	_scope_stack.pop_back();

	for(auto s : _current_scope.symbols) DEBUG_PRINT_F_MSG("    kept %s -> %s", 
		s.first.c_str(), s.second->get_ident().c_str());
	for (auto s = _scope_stack.rbegin(); s != _scope_stack.rend(); s++)
		for(auto ss : s->symbols) DEBUG_PRINT_F_MSG("    kept %s -> %s", 
			ss.first.c_str(), ss.second->get_ident().c_str());
}

void Parser::synchronize()
{
	_panic_mode = false;
	
	while(!is_at_end() && _current.type != TOKEN_NEWLINE) advance();
}

// ===================== expressions ====================

void Parser::assignment()
{
	// get target
	Target target = consume_target();
	if(target.invalid && !consume(TOKEN_EQUAL, "Expected assignment.")) return;

	DEBUG_PRINT_NL();
	DEBUG_PRINT_F_MSG("assigning '%s'", target.name.c_str());

	// add arguments as symbols
	scope_up();
	if(target.has_params) for(auto p : target.params)
	{
		Target t{
			.token = target.token,
			.name = p,
			.has_params = false,
			.params = {},
			.invalid = false,
		};
		set_symbol(Symbol{
			.target = t,
			.id = 0, // will be set by set_symbol()
			.body = nullptr,
			.invalid = false,
		});
	}

	// get expression
	ExprNode* body = expression();
	scope_down();

	DEBUG_PRINT_F_MSG("assigned '%s'", target.name.c_str());
	set_symbol(Symbol{
		.target = target,
		.id = 0,
		.body = body,
		.invalid = target.invalid || !body
	});
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

	// variables and calls
	else if(match(TOKEN_IDENTIFIER))
	{
		if(check(TOKEN_LEFT_PAREN)) return call();
		else return variable();
	}

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
	// TODO: check the variable's existence and whatnot
	return new VariableNode(_previous, PREV_TOKEN_STR);
}

CallNode* Parser::call()
{
	Token tok = _previous;
	string name = PREV_TOKEN_STR;

	CONSUME_OR_RET_NULL(TOKEN_LEFT_PAREN, "Expected '(' after identifier.");

	Symbol* symbol = get_symbol(name);
	if(!symbol)
	{
		error_at(&tok, "Function does not exist.");
		return nullptr;
	}
	if(!symbol->target.has_params)
	{
		HOLD_PANIC();
		error_at(&tok, "Symbol is not a function.");
		if(!PANIC_HELD) note_declaration("Symbol", name, &symbol->target.token);
		return nullptr;
	}
	DEBUG_PRINT_F_MSG("callee: '%s.%u'", symbol->target.name.c_str(), symbol->id);

	vector<ExprNode*> args;
	int arity = symbol->target.params.size();
	
	if(!check(TOKEN_RIGHT_PAREN)) do args.push_back(expression()); while(match(TOKEN_COMMA));

	if(args.size() != arity)
	{
		HOLD_PANIC();
		error_at(&tok, tools::fstr("Expected %d argument%s, but %d were given.",
			arity, arity == 1 ? "" : "s", args.size()));
		if(!PANIC_HELD) note_declaration("Function", name, &symbol->target.token);
		return nullptr;
	}

	CONSUME_OR_RET_NULL(TOKEN_RIGHT_PAREN, "Expected ')' after arguments.");

	// return funcprops.invalid ? nullptr : new CallNode(tok, name, args, funcprops.ret_type, lexparams, paramscount);
	return new CallNode(tok, symbol->get_ident(), args);
}

// ======================= misc. =======================

Status Parser::parse(string infile, CCP source, vector<Symbol>* symbols_dest)
{
	// print_tokens_from_src(source);

	// set members
	_scanner = Scanner(&infile, source);
	_symbols = vector<Symbol>();
	_scope_stack = vector<Scope>();
	_current_scope = Scope{map<string, Symbol*>()};

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
	*symbols_dest = _symbols;
	return _had_error ? STATUS_PARSE_ERROR : STATUS_SUCCESS;
}