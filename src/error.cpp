#include "error.hpp"
#include "tools.hpp"

void ErrorDispatcher::print_token_marked(Token *token, CCP color)
{
	string tokenline = "";
	string markerline = "";

	// get offset of token (first char)
	ptrdiff_t token_offset = token->start - token->source;

	// find first newline before token
	ptrdiff_t tok_ln_begin;
	{
		tok_ln_begin = token_offset;
		if(token->line == 1)
		{
			tok_ln_begin = 0;
		}
		else
		{
			while(tok_ln_begin > 0 && token->source[tok_ln_begin] != '\n') tok_ln_begin--;
			tok_ln_begin++; // skip newline itself
		}
	}

	// find first newline after token
	ptrdiff_t tok_ln_end = token_offset + token->length;
	while(token->source[tok_ln_end] != '\n' && token->source[tok_ln_end] != '\0') tok_ln_end++;

	ptrdiff_t tok_ln_before_tok_len; // keep for later use

	// get line with marked token
	{
		/*
			Inserting the escape codes for coloring won't work, so instead we find
			length A and B as seen below:

				this is an example line with a WRONG token in it.
				<------------- A ------------->     <---- B ---->

			And use those and the info we have of the token to merge them,
			and the escape codes together in a new string.
		*/

		// find string on the token's line before and after the token itself
		tok_ln_before_tok_len = token_offset - tok_ln_begin;

		string tok_ln_before_tok = string(token->source + tok_ln_begin, tok_ln_before_tok_len);

		ptrdiff_t tok_ln_after_tok_len = tok_ln_end - token_offset - token->length;
		string tok_ln_after_tok = string(token->source + token_offset + token->length, tok_ln_after_tok_len);

		// get token and its full line
		string tok = string(token->start, token->length);
		string tok_ln = tok_ln_before_tok + color + tok + COLOR_NONE + tok_ln_after_tok;
		tokenline = tools::fstr(" %3d| %s", token->line, tok_ln.c_str());
	}

	// get the marker line
	{
		/*
			Example:

			 69| @this i32 WRONG ();
			               ^~~~~
		*/

		int prefixlen = tools::fstr(" %3d| ", token->line).length();
		markerline +=  string(prefixlen, ' ');

		for(int i = 0; i < tok_ln_before_tok_len; i++)
			markerline +=  token->source[tok_ln_begin + i] == '\t' ? '\t' : ' ';

		markerline += string(color) + "^";
		markerline += string(token->length - 1, '~');

		markerline += COLOR_NONE;
	}

	// print it all out
	cerr << tokenline << endl;
	cerr << markerline << endl;
}

void ErrorDispatcher::print_line_marked(uint line_no, string line, CCP color)
{
	string prefix = tools::fstr(" %3d| ", line_no);

	cerr << prefix + line << endl;

	cerr << COLOR_RED;
	cerr << string(prefix.length(), ' ') + ('^' + string(line.length() - 1, '~'));
	cerr << COLOR_NONE;
	cerr << endl;
}

void ErrorDispatcher::__dispatch(CCP color, CCP prompt, CCP message)
{
	cerr << tools::fstr("[solve] %s%s" COLOR_NONE ": %s",
						color, prompt, message) << endl;
}

void ErrorDispatcher::__dispatch_at_token(CCP color, Token* token, CCP prompt, CCP message)
{
	fprintf(stderr, "[%s:%d:%u] %s%s" COLOR_NONE ": %s\n",
			token->file->c_str(),
			token->line, get_token_col(token) + 1, 
			color, prompt, message);
}

// if line == 0 lineno is omitted. likewise with filename
void ErrorDispatcher::__dispatch_at_line(CCP color, uint line, CCP filename, CCP prompt, CCP message)
{
	if(line) fprintf(stderr, "[%s:%d] %s%s" COLOR_NONE ": %s\n",
				filename ? filename : "???", line, color, prompt, message);

	else fprintf(stderr, "[%s] %s%s" COLOR_NONE ": %s\n",
				filename ? filename : "???", color, prompt, message);
}


void ErrorDispatcher::error(CCP prompt, CCP message)
	{ __dispatch(COLOR_RED, prompt, message); }

void ErrorDispatcher::error_at_line(uint line, CCP filename, CCP prompt, CCP message)
	{ __dispatch_at_line(COLOR_RED, line, filename, prompt, message); }

void ErrorDispatcher::error_at_token(Token* token, CCP prompt, CCP message)
	{ __dispatch_at_token(COLOR_RED, token, prompt, message); }

void ErrorDispatcher::warning(CCP prompt, CCP message)
	{ __dispatch(COLOR_PURPLE, prompt, message); }

void ErrorDispatcher::warning_at_line(uint line, CCP filename, CCP prompt, CCP message)
	{ __dispatch_at_line(COLOR_PURPLE, line, filename, prompt, message); }

void ErrorDispatcher::warning_at_token(Token* token, CCP prompt, CCP message)
	{ __dispatch_at_token(COLOR_PURPLE, token, prompt, message); }

void ErrorDispatcher::note(CCP message)
	{ __dispatch(COLOR_GREEN, "Note", message); }

void ErrorDispatcher::note_at_line(uint line, CCP filename, CCP message)
	{ __dispatch_at_line(COLOR_GREEN, line, filename, "Note", message); }

void ErrorDispatcher::note_at_token(Token* token, CCP message)
	{ __dispatch_at_token(COLOR_GREEN, token, "Note", message); }
