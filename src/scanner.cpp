#include "common.hpp"
#include "scanner.hpp"
#include "tools.hpp"

Scanner::Scanner() {}

Scanner::Scanner(string* filename, const char *source)
{
	_filename = filename;
	_src_start = source;
	_start = source;
	_current = source;
	_line = 1;
}

int Scanner::getScannedLength()
{
	//
	return (int)(_current - _src_start);
}

bool Scanner::isAtEnd()
{
	//
	return *_current == '\0';
}

bool Scanner::isDigit(char c)
{
	//
	return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c)
{
	return (c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z') ||
		    c == '_';
}

bool Scanner::match(char expected)
{
	if (isAtEnd())
		return false;
	if (*_current != expected)
		return false;
	_current++;
	return true;
}

char Scanner::advance()
{
	_current++;
	return _current[-1];
}

char Scanner::peek()
{
	//
	return *_current;
}

char Scanner::peekNext()
{
	if (isAtEnd())
		return '\0';
	return _current[1];
}

Token Scanner::makeToken(TokenType type)
{
	return Token{
		/*type*/ type,
		/*source*/ _src_start,
		/*start*/ _start,
		/*length*/ (int)(_current - _start),
		/*line*/ _line,
		/*file*/ _filename,
	};
}

Token Scanner::errorToken(const char *message)
{
	return Token{
		/*type*/ TOKEN_ERROR,
		/*source*/ _src_start,
		/*start*/ message,
		/*length*/ (int)strlen(message),
		/*line*/ _line,
		/*file*/ _filename
	};
}

Token Scanner::number()
{
	if(isAlpha(peek())) // other notation
	{
		// assert proper notation
		char last = tolower(advance());
		if(last != 'b' && last != 'c' && last != 'x') return errorToken("Invalid numerical notation.");

		// set max char
		char max = last == 'b' ? '1'
				 : last == 'c' ? '7'
				 : /*last=='x'*/ 'f';

		while(isDigit(peek()) || isAlpha(peek()))
			if(advance() > max) return errorToken("Invalid numerical notation.");
			
		return makeToken(TOKEN_INTEGER);
	}
	else // decimal (possibly float)
	{
		while (isDigit(peek())) advance();

		// Look for a fractional part.
		if (peek() == '.' && isDigit(peekNext()))
		{
			// Consume the ".".
			advance();

			while (isDigit(peek())) advance();

			return makeToken(TOKEN_FLOAT);
		}
		return makeToken(TOKEN_INTEGER);
	}

}

Token Scanner::identifier()
{
	while(peek() == '\'') advance();

	if(!isAlpha(peek())) return errorToken("Invalid identifier.");

	while (isAlpha(peek()) || isDigit(peek())) advance();

	return makeToken(TOKEN_IDENTIFIER);
}

void Scanner::skipWhitespaces()
{
	for (;;)
	{
		char c = peek();
		switch (c)
		{
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		// case '\n':
		// 	_line++;
		// 	advance();
		// 	break;
		case ';':
			if (peekNext() == ';')
			{
				for (;;)
				{
					if (peek() == ';' && peekNext() == ';')
					{
						advance();
						advance();
						break;
					}
					else if (isAtEnd()) break;
					else if (peek() == '\n') _line++;
					advance();
				}
			}
			else
			{
				while (peek() != '\n' && !isAtEnd())
					advance();
			}
			break;
		default:
			return;
		}
	}
}

Token Scanner::scanToken()
{
	skipWhitespaces();

	_start = _current;

	if (isAtEnd())
		return makeToken(TOKEN_EOF);

	char c = advance();

	// check for idents
	if (isAlpha(c) || c == '\'')
		return identifier();
	// check for digits
	if (isDigit(c))
		return number();

	switch (c)
	{
		// single-character

		case '(': return makeToken(TOKEN_LEFT_PAREN);
		case ')': return makeToken(TOKEN_RIGHT_PAREN);
		case ',': return makeToken(TOKEN_COMMA);
		case '+': return makeToken(TOKEN_PLUS);
		case '-': return makeToken(TOKEN_MINUS);
		case '*': return makeToken(TOKEN_STAR);

		// two-character
		case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL	: TOKEN_EQUAL);
		case '/': return makeToken(match('=') ? TOKEN_SLASH_EQUAL	: TOKEN_SLASH);
		case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL	: TOKEN_LESS);
		case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

		case '\n':
		{
			_line++;
			while(peek() == '\n') advance();
			return makeToken(TOKEN_NEWLINE);
		}
	}

	char* errstr = new char[25]; // just above what's needed
	strcpy(errstr, "Unexpected character 'X'.");
	errstr[strlen(errstr) - 3] = c;
	return errorToken(errstr);
}

// =========================

char *get_tokentype_str(TokenType type)
{
	switch(type)
	{
		// Single-character tokens.
		case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
		case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
		// case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
		// case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
		// case TOKEN_LEFT_B_BRACE: return "LEFT_B_BRACE";
		// case TOKEN_RIGHT_B_BRACE: return "RIGHT_B_BRACE";
		case TOKEN_SLASH: return "SLASH";
		case TOKEN_COMMA: return "COMMA";
		// case TOKEN_SEMICOLON: return "SEMICOLON";
		case TOKEN_STAR: return "STAR";
		// case TOKEN_MODULO: return "MODULO";

		// One or two character tokens.
		case TOKEN_PLUS: return "PLUS";
		case TOKEN_MINUS: return "MINUS";
		case TOKEN_GREATER: return "GREATER";
		case TOKEN_LESS: return "LESS";
		case TOKEN_EQUAL: return "EQUAL";

		// Multi-character tokens
		case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
		case TOKEN_SLASH_EQUAL: return "SLASH_EQUAL";
		case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
		case TOKEN_LESS_EQUAL: return "LESS_EQUAL";

		// Literals.
		case TOKEN_IDENTIFIER: return "IDENTIFIER";
		case TOKEN_INTEGER: return "INTEGER";
		case TOKEN_FLOAT: return "FLOAT";

		// misc.
		case TOKEN_NEWLINE: return "NEWLINE";
		case TOKEN_ERROR: return "ERROR";
		case TOKEN_EOF: return "EOF";
	}
}

void print_tokens_from_src(const char *src)
{
	Scanner scanner(new string("<none>"), src);
	
	Token token;
	do {
		token = scanner.scanToken();
		printf("%d: \"%.*s\" \t\t-> %s\n\n",
			token.line, token.length, token.start, get_tokentype_str(token.type));

	} while (token.type != TOKEN_EOF);
}