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

	ABORT(STATUS_PARSE_ERROR);
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

	if(!consume(TOKEN_IDENTIFIER, "Expected symbol target.")) return target;
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
	
	target.invalid = _panic_mode;
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

int Parser::parse_prev_integer()
{
	int base = 0;
	string tok = PREV_TOKEN_STR;
	
			if(tok.size() > 2 && tolower(tok[1]) == 'b') base = 2;
	else if(tok.size() > 2 && tolower(tok[1]) == 'c') base = 8;
	else if(tok.size() > 2 && tolower(tok[1]) == 'x') base = 16;
	else base = 10;

	return strtol(tok.c_str() + (base != 10 ? 2 : 0), NULL, base);
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
	if(symbol.id == 0)
	{
		for(auto s = _env.symbols.rbegin(); s != _env.symbols.rend(); s++)
			if((*s)->target.name == symbol.target.name)
			{
				symbol.id = (*s)->id + 1;
				break;
			}
	}

	Symbol* symptr = new Symbol(symbol);
	if(symbol.id >= 0) _env.symbols.push_back(symptr);
	_current_scope.symbols[symbol.target.name] = symptr;

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

// id = -1 is ignored
Symbol* Parser::get_symbol(string name, int id)
{
	if(id == -1)
	{
		if(_current_scope.symbols.find(name) != _current_scope.symbols.end())
			return _current_scope.symbols.at(name);

		for (auto s = _scope_stack.rbegin(); s != _scope_stack.rend(); s++)
			if (s->symbols.find(name) != s->symbols.end())
				return s->symbols.at(name);
	}
	else for(auto s : _env.symbols)
		if(s->id == id && s->target.name == name) return s;

	return nullptr;
}

bool Parser::check_symbol(string name)
{
	return get_symbol(name) != nullptr;
	// return !get_symbol(name).invalid;
}

void Parser::scope_up()
{
	// DEBUG_PRINT_MSG("scope up");
	_scope_stack.push_back(_current_scope);
	_current_scope = Scope{map<string, Symbol*>()};
}

void Parser::scope_down()
{
	// DEBUG_PRINT_MSG("scope down");
	// for(auto s : _current_scope.symbols) DEBUG_PRINT_F_MSG("    dump %s -> %s", 
	// 	s.first.c_str(), s.second->get_ident().c_str());

	_current_scope = _scope_stack[_scope_stack.size() - 1];
	_scope_stack.pop_back();

	// for(auto s : _current_scope.symbols) DEBUG_PRINT_F_MSG("    kept %s -> %s", 
	// 	s.first.c_str(), s.second->get_ident().c_str());
	// for (auto s = _scope_stack.rbegin(); s != _scope_stack.rend(); s++)
	// 	for(auto ss : s->symbols) DEBUG_PRINT_F_MSG("    kept %s -> %s", 
	// 		ss.first.c_str(), ss.second->get_ident().c_str());
}

// ===================== some shit -====================

void Parser::bind()
{
	uint addr = parse_prev_integer();

	if(!consume(TOKEN_ARROW, "Expected '->' after bind address.")) return;		

	BoundValue val; // get value
	{
		if(match(TOKEN_INTEGER) || match(TOKEN_FLOAT))
		{
			NumberNode* numnode = number();
			val.as.num = numnode->_value;
			val.type = BoundValue::NUMBER;
			delete numnode;
		}
		else if(match(TOKEN_STRING))
		{
			val.as.str = strndup(_previous.start + 1, _previous.length - 2);
			val.type = BoundValue::STRING;
		}
		else error_at_current("Expected bindable value.");
	}

	_env.bindings[addr] = val;
}

void Parser::assignment()
{
	// get target
	Target target = consume_target();
	if(target.invalid || !consume(TOKEN_EQUAL, "Expected assignment.")) return;

	DEBUG_PRINT_NL();
	scope_up();

	// add params as symbols
	for(int i = 0; i < target.params.size(); i++)
	{
		Target t{
			.token = target.token,
			.name = target.params[i],
			.has_params = false,
			.params = {},
			.invalid = false,
		};
		set_symbol(Symbol{
			.target = t,
			.id = -i - 1, // will be set by set_symbol()
			.body = nullptr,
			.invalid = false,
		});
	}

	// get expression
	ExprNode* body = expression();

	scope_down();
	set_symbol(Symbol{
		.target = target,
		.id = 0,
		.body = body,
		.invalid = target.invalid || _panic_mode
	});

	DEBUG_PRINT_F_MSG("assigned '%s'", target.name.c_str());
}

// ===================== expressions ====================

ExprNode* Parser::expression()
{
	//
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
	if(match(TOKEN_INTEGER) || match(TOKEN_FLOAT)) return number();

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
		Token tok = _previous;
		string name = PREV_TOKEN_STR;

		// allow explicit ID
		int id = -1;
		if(match(TOKEN_DOT))
		{
			if(!consume(TOKEN_INTEGER, "Expect ID after '.'.")) return nullptr;
			id = parse_prev_integer();
		}

		Symbol* symbol = get_symbol(name, id);
		if(!symbol)
		{
			error_at(&tok, "Symbol does not exist.");
			return nullptr;
		}

		if(match(TOKEN_LEFT_PAREN)) return finish_call(tok, symbol);
		else return finish_variable(tok, symbol);
	}

	// actions
	else if(match(TOKEN_AT)) return action();

	error_at_current("Expected expression.");
	return nullptr;
}

// ===================== primaries =====================

NumberNode* Parser::number()
{
	switch(_previous.type)
	{
		case TOKEN_INTEGER:
		{
			int intval = parse_prev_integer();
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

VariableNode* Parser::finish_variable(Token tok, Symbol* symbol)
{
	string name = string(tok.start, tok.length);

	if(symbol->target.has_params)
	{
		HOLD_PANIC();
		error_at(&tok, "Symbol is not a variable.");
		if(!PANIC_HELD) note_declaration("Symbol", name, &symbol->target.token);
		return nullptr;
	}

	DEBUG_PRINT_F_MSG("variable: '%s'", symbol->get_ident().c_str());

	if(symbol->invalid) DEBUG_PRINT_MSG("VARIABLE INVALID!");
	return symbol->invalid ? nullptr : new VariableNode(_previous, symbol);
}

CallNode* Parser::finish_call(Token tok, Symbol* symbol)
{
	string name = string(tok.start, tok.length);

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

	if(symbol->invalid) DEBUG_PRINT_MSG("CALLEE INVALID!");
	return symbol->invalid ? nullptr : new CallNode(tok, symbol, args);
}

ActionNode* Parser::action()
{
	CONSUME_OR_RET_NULL(TOKEN_IDENTIFIER, "Expected identifier after '@'.");
	Token tok = _previous;
	Action* action = get_action(PREV_TOKEN_STR);

	if(!action) { error("Action does not exist."); return nullptr; }

	CONSUME_OR_RET_NULL(TOKEN_LEFT_B_BRACE, "Expected '['.");

	vector<ExprNode*> args;
	if(!check(TOKEN_RIGHT_B_BRACE)) do args.push_back(expression()); while(match(TOKEN_COMMA));

	if(args.size() != action->arity)
	{
		error_at(&tok, tools::fstr("Expected %d argument%s, but %d were given.",
			action->arity, action->arity == 1 ? "" : "s", args.size()));
		return nullptr;
	}

	CONSUME_OR_RET_NULL(TOKEN_RIGHT_B_BRACE, "Expected ']' after arguments.");
	return new ActionNode(tok, action, args);
}

// ======================= misc. =======================

Status Parser::parse(string infile, CCP source, Environment* env)
{
	// set members
	_scanner = Scanner(new string(infile), source);
	_scope_stack = vector<Scope>();
	_current_scope = Scope{map<string, Symbol*>()};

	_env = {};

	_had_error = false;
	_panic_mode = false;
	_error_dispatcher = ErrorDispatcher();

	_main_file = infile;

	advance();
	while (!is_at_end())
	{
		if(match(TOKEN_INTEGER)) bind();
		else assignment();
		// if(_panic_mode) synchronize();
	}

	DEBUG_PRINT_NL();
	DEBUG_PRINT_MSG("Parsing complete!");
	
	*env = _env;
	return _had_error ? STATUS_PARSE_ERROR : STATUS_SUCCESS;
}