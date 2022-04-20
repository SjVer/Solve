#define ARGP_NO_EXIT
#define ARGP_NO_HELP

#include <argp.h>
#include <regex>

#include "common.hpp"
#include "tools.hpp"

#include "ast.hpp"
#include "parser.hpp"
#include "visualizer.hpp"

// ================= arg stuff =======================

const char *argp_program_version = APP_NAME " " APP_VERSION;
const char *argp_program_bug_address = EMAIL;
static char args_doc[] = "file...";

struct arguments
{
	char *infile = nullptr;
	int verbose = 0;
	bool generate_ast = false;
};

#define ARG_GEN_AST 1

static struct argp_option options[] =
{
	{"help", 				'h', 			 0, 		  0, "Display a help message."},
	{"version", 			'V', 			 0, 		  0, "Display compiler version information."},
	{"usage", 				'u', 			 0, 		  0, "Display a usage information message."},
	{"verbose", 			'v', 			 0, 		  0, "Produce verbose output."},
	{"generate-ast",  		ARG_GEN_AST, 	 0, 		  0, "Generate AST image."},

	{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static char *doc = strdup(tools::fstr(
	APP_DOC, APP_NAME, EMAIL, LINK, __DATE__, __TIME__, OS_NAME, COMPILER
	).c_str());
static struct argp argp = {options, parse_opt, args_doc, doc};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = (struct arguments*)state->input;

	switch (key)
	{
	case 'h':
		argp_help(&argp, stdout, ARGP_HELP_STD_HELP, APP_NAME);
		exit(0);
	case 'V':
		cout << argp_program_version << endl;
		exit(0);
	case 'u':
		argp_usage(state);
		exit(0);
	case 'v':
		arguments->verbose += 1;
		break;
	case ARG_GEN_AST:
		arguments->generate_ast = true;
		break;

	case ARGP_KEY_ARG:
	{
		if(arguments->infile)
		{
			ERR("Cannot interpret more than 1 file.");
			ABORT(STATUS_CLI_ERROR);
		}
		else arguments->infile = arg;
		break;
	}
	case ARGP_KEY_END:
	{
		if(!arguments->infile) argp_usage(state);
		break;
	}
	default: return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// ================================

int main(int argc, char **argv)
{
	// ========= argument stuff =========

	struct arguments arguments;

	/* Where the magic happens */
	if(argp_parse(&argp, argc, argv, 0, 0, &arguments)) ABORT(STATUS_CLI_ERROR);

	// ===================================

	Status status = STATUS_SUCCESS;
	CCP source = strdup(tools::readf(arguments.infile).c_str());
	vector<Symbol*> symbols;

	#define ABORT_IF_UNSUCCESSFULL() if(status != STATUS_SUCCESS) ABORT(status)

	// parse program
	Parser* parser = new Parser();
	status = parser->parse(arguments.infile, source, &symbols);
	ABORT_IF_UNSUCCESSFULL();


	// print symbols
	if(arguments.verbose)
	{
		MSG("Defined symbols:");
		for(auto s : symbols)
		{
			string msg = "    " + s->get_ident();
			if(s->target.has_params)
			{
				msg += " (";
				for(auto p : s->target.params) msg += p + (p != s->target.params.back() ? ", " : "");
				msg += ")";
			}
			MSG(msg);
		}
	}

	// generate visualization
	if(arguments.generate_ast)
	{
		ASTVisualizer viz = ASTVisualizer();

		viz.init();
		for(auto s : symbols) viz.visualize(string(arguments.infile) + ".svg", s);
		viz.finalize();

		MSG("AST image written to \"" + string(arguments.infile) + ".svg\".");
		exit(STATUS_SUCCESS);
	}


	// execute
	// Parser* parser = new Parser();
	// status = parser->parse(arguments.infiles[0], source, &astree);
	// ABORT_IF_UNSUCCESSFULL();

	free((void*)source);
	DEBUG_PRINT_MSG("Exited sucessfully.");
	return STATUS_SUCCESS;
}