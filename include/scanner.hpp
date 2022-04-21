#ifndef SCANNER_H
#define SCANNER_H

#include "pch"

using namespace std;

typedef enum
{
	// Single-character tokens.
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	// TOKEN_LEFT_BRACE,
	// TOKEN_RIGHT_BRACE,
	TOKEN_LEFT_B_BRACE,
	TOKEN_RIGHT_B_BRACE,
	TOKEN_EQUAL,
	TOKEN_COMMA,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_SLASH,
	TOKEN_GREATER,
	TOKEN_LESS,
	// TOKEN_MODULO,

	// Multi-character tokens
	TOKEN_EQUAL_EQUAL,
	TOKEN_SLASH_EQUAL,
	TOKEN_GREATER_EQUAL,
	TOKEN_LESS_EQUAL,
	TOKEN_ARROW,

	// Literals.
	TOKEN_AT,
	TOKEN_IDENTIFIER,
	TOKEN_INTEGER,
	TOKEN_FLOAT,
	TOKEN_STRING,

	// misc.
	TOKEN_NEWLINE,
	TOKEN_ERROR,
	TOKEN_EOF
} TokenType;

typedef struct
{
	TokenType type;
	const char *source;
	const char *start;
	int length;
	int line;
	string* file;
} Token;

class Scanner
{
public:
	Scanner();
	Scanner(std::string* filename, const char *source);
	Token scanToken();
	int getScannedLength();

private:
	const char *_src_start;	
	const char *_start;
	const char *_current;
	int _line;

	std::string* _filename;

	bool isAtEnd();
	bool isDigit(char c);
	bool isAlpha(char c);
	bool match(char expected);
	char advance();
	char peek();
	char peekNext();
	Token makeToken(TokenType type);
	Token errorToken(const char *message);
	Token number();
	Token string();
	Token identifier();
	void skipWhitespaces();
};

/*
static uint get_token_col(Token* token, int tab_width = -1)
{
	if(token->type == TOKEN_ERROR) return 0;
	
	// get offset of token (first char)
	ptrdiff_t token_offset = token->start - token->source;

	// find first newline before token
	ptrdiff_t tok_ln_begin = token_offset;
	while(tok_ln_begin > 0 && token->source[tok_ln_begin] != '\n') tok_ln_begin--;
	tok_ln_begin++; // skip newline itself

	ptrdiff_t col = (token_offset - tok_ln_begin);

	// if tab width set account for that
	for(int i = -col; tab_width >= 0 && i < 0; i++)
		if(token->start[i] == '\t') col += tab_width;
	
	return col >= 0 ? col : 0;
}
*/

#endif