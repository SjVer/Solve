#ifndef SOLVE_COMMON_H
#define SOLVE_COMMON_H

#pragma once

#include "pch"

using namespace std;

// c++ compiler name
#ifndef COMPILER
#define COMPILER "unknown"
#endif

// OS_NAME specific
#pragma region
#ifdef _WIN32
	#define OS_NAME "Windows 32-bit"
	#define PATH_SEPARATOR '\\'
#elif _WIN64
	#define OS_NAME "Windows 64-bit"
	#define PATH_SEPARATOR '\\'
#elif __APPLE__ || __MACH__
	#define OS_NAME "Mac OS_NAMEX"
	#define PATH_SEPARATOR '/'
#elif __linux__
	#define OS_NAME "Linux"
	#define PATH_SEPARATOR '/'
#elif __unix || __unix__
	#define OS_NAME "Unix"
	#define PATH_SEPARATOR '/'
#else
	#define OS_NAME "Unknown OS"
	#define PATH_SEPARATOR '/'
#endif
#pragma endregion

#define COLOR_RED "\x1b[1;31m"
#define COLOR_GREEN "\x1b[0;32m"
#define COLOR_PURPLE "\x1b[1;35m"
#define COLOR_NONE "\x1b[0m"
#define COLOR_BOLD "\x1b[1m"

// app info
#pragma region
#define APP_NAME "solve"
#define APP_VERSION "0.0.1"
#define APP_DOC "%s -- The Solve Interpreter.\nWritten by Sjoerd Vermeulen (%s)\v\
More information at %s.\nBuild: %s %s on %s (%s)."
// format: APP_NAME, EMAIL, LINK, __DATE__, __TIME__, OS_NAME, COMPILER
#define EMAIL "sjoerd@marsenaar.com"
#define LINK "???"

// version info
#define APP_VERSION_0 0
#define APP_VERSION_1 0
#define APP_VERSION_2 1

#pragma endregion

// macros
#pragma region
#define CCP const char*
#define STRINGIFY(value) #value
#define LINE_MARKER_REGEX " ([0-9]+) \"(.+)\""
// #define FLAG_MARKER_REGEX " f ([0-9]+)"

#ifdef DEBUG
#define __DEBUG_MARKER(file, line) "[debug:" file ":" STRINGIFY(line) "]"
#define DEBUG_MARKER __DEBUG_MARKER(__FILE__, __LINE__)
#define DEBUG_PRINT_LINE() cout << tools::fstr(\
	DEBUG_MARKER " line %d passed!",__LINE__) << endl
#define DEBUG_PRINT_VAR(value, formatspec) cout << tools::fstr(\
	DEBUG_MARKER " var %s = " #formatspec, #value, value) << endl
#define DEBUG_PRINT_MSG(msg) cout << DEBUG_MARKER " " msg << endl;
#define DEBUG_PRINT_F_MSG(format, ...) cout <<  tools::fstr( \
	DEBUG_MARKER " " format, __VA_ARGS__) << endl
#else
#define DEBUG_MARKER {}
#define DEBUG_PRINT_LINE() {}
#define DEBUG_PRINT_VAR(value, formatspec) {}
#define DEBUG_PRINT_MSG(msg) {}
#define DEBUG_PRINT_F_MSG(format, ...) {}
#endif

#define ABORT(status) { cerr << tools::fstr("[solve] Aborted with code %d.\n", status); exit(status); }
#define THROW_INTERNAL_ERROR(where) { DEBUG_PRINT_MSG("internal error " where); raise(SIGINT); }

#pragma endregion

// status enum
typedef enum
{
	STATUS_SUCCESS = 0,
	STATUS_CLI_ERROR = 1,
	STATUS_PARSE_ERROR = 2,
	STATUS_INTERPRET_ERROR = 3,

	STATUS_INTERNAL_ERROR = -1
} Status;

#endif