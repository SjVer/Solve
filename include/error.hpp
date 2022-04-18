#ifndef ERROR_H
#define ERROR_H

#include "common.hpp"
#include "scanner.hpp"

class ErrorDispatcher
{
    private:

    void __dispatch(CCP color, CCP prompt, CCP message);
    void __dispatch_at_token(CCP color, Token *token, CCP prompt, CCP message);
    void __dispatch_at_line(CCP color, uint line, CCP filename, CCP prompt, CCP message);    

    public:

    ErrorDispatcher() {}

    void print_token_marked(Token *token, CCP color);
    void print_line_marked(uint line_no, string line, CCP color);

    void error(CCP prompt, CCP message);
    void error_at_token(Token *token, CCP prompt, CCP message);
    void error_at_line(uint line, CCP filename, CCP prompt, CCP message);

    void warning(CCP prompt, CCP message);
    void warning_at_token(Token *token, CCP prompt, CCP message);
    void warning_at_line(uint line, CCP filename, CCP prompt, CCP message);

    void note(CCP message);
    void note_at_token(Token *token, CCP message);
    void note_at_line(uint line, CCP filename, CCP message);
};

#endif