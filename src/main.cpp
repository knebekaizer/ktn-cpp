#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "cmdargs.hpp"

#include "parser_tree.h"

#include "trace.h"

using namespace std;

namespace {
vector<string> GetFilesToProcess(CmdArgs &cmd_args, int &argc, char **&argv)
{
	vector<string> files;
	bool wtf = false;
	try {
		--argc;
		++argv;
		files = cmd_args.Consume(argc, argv);
		if (files.empty()) {
			cerr << "No input files specified." << endl;
			wtf = true;
		}
	}
	catch (const CmdArgs::Exception &error) {
		cerr << "Error: " << error.GetError() << endl;
		wtf = true;
	}

	if (wtf) {
		cout << "Reflang tool to generate reflection metadata.\n";
		cout << "\n";
		cout << "Usage: reflang [reflang_flags] -- [clang_flags]\n";
		cout << "Where [reflang_flags] are any of the below, and [clang_flags] "
		        "are any flags supported by the libclang version installed\n";
		cout << "\n";
		cout << "Supported flags:\n";
		cmd_args.PrintHelp();
		exit(-1);
	}

	return files;
}
}

#ifdef USE_RUNTIME_LOG_LEVEL
LOG_LEVEL::LOG_LEVEL gLogLevel = DEF_LOG_LEVEL;
#endif

#include "cxxopts.hpp"

std::string joinToString(const std::vector<std::string>& v, std::string sep = " ") {
	ostringstream os;
	auto n = v.size();
	for (auto& x : v)  os << x << (--n ? sep : "");
	return os.str();
}

using namespace cxxopts;

class Config {
public:
	Config(int argc, char **argv);

	std::vector<std::string> files;
	std::vector<std::string> parse_options;
};

Config::Config(int argc, char **argv) {
	cxxopts::Options options(argv[0], " - AST parser skeleton");

	options
		//	.allow_unrecognised_options()
			.add_options()
					("i,input", "Input files or \"-\" for stdin", value<std::vector<std::string>>()->default_value({"-"}))
					("o,output", "Output file, default is stdout", cxxopts::value<std::string>()->default_value("-"))
					("I,include", "Include path", value<std::vector<std::string>>(), "PATH")
					("P,parse-options", "Compiler options to be directly sent to the clang parser", value<std::vector<std::string>>())
					("l,log-level", "Log level, as digit or word [0:none, 1:fatal, 2:error, 3:warn, 4:info, 5:debug, 6:trace]", cxxopts::value<std::string>()->default_value("info"))
					("help", "Print help");

	options.parse_positional({"input"});

	auto result = options.parse(argc, argv);

	if (result.count("help")) {
		std::cout << options.help({"", "Group"}) << std::endl;
		exit(0);
	}

	if (result.count("log-level")) {
#ifdef USE_RUNTIME_LOG_LEVEL
		std::vector<std::string> opts = {"0", "none", "1", "fatal", "2", "error", "3", "warn", "4", "info", "5", "debug", "6", "trace"};
		auto found = std::find(opts.begin(), opts.end(), result["log-level"].as<std::string>());
		if (found != opts.end()) {
			unsigned int level = (found - opts.begin()) / 2;
			assert(level < LOG_LEVEL::LOG_LEVEL::invalid);
			gLogLevel = (LOG_LEVEL::LOG_LEVEL)level;
		}
#else
		log_warn << "command line option \"--log-level\" will be ignored because compile-time option USE_RUNTIME_LOG_LEVEL was not enabled";
#endif
	}

	files = result["input"].as<std::vector<std::string>>();

	if (result.count("include")) {
		parse_options = result["include"].as<std::vector<std::string>>();
	}
	if (result.count("parse-options")) {
		auto const& r = result["parse-options"].as<std::vector<std::string>>();
		parse_options.insert(parse_options.end(), r.begin(), r.end());
	}

	log_trace << "files:\n    " << joinToString(files, "\n    ");
	log_trace << "parse_options:\n    " << joinToString(parse_options, "\n    ");
}

int main(int argc, char **argv)
{
    TraceX(__clang_version__);

    try {
    	Config config(argc, argv);
    	exit(0);
    } catch (const cxxopts::OptionException &e) {
	    std::cerr << "error parsing options: " << e.what() << std::endl;
	    exit(131);
    } catch (const std::exception& e) {
    	log_fatal << "General exception " << e.what();
    	exit(130);
    } catch (...) {
	    log_fatal << "Unknown exception";
	    exit(129);
    }

CmdArgs cmd_args;
	auto log_level = cmd_args.Register<int>(
			"--log-level",
			"0:none, 1:fatal, 2:error, 3:warn, 4:info, 5:debug, 6:trace",
			-1);
	auto list_only = cmd_args.Register<bool>(
			"--list-only",
			"Only list type names, don't generate",
			false);
	auto path_filter = cmd_args.Register<string>(
			"--headers",
			"path to headers (wildcard) eligible for generation",
			".*");
	auto filter_include = cmd_args.Register<string>(
			"--include",
			"regex for which types to include in generation",
			".*");
	auto filter_exclude = cmd_args.Register<string>(
			"--exclude",
			"regex for which types to exclude from generation",
			"std::.*");
	auto reflang_include = cmd_args.Register<string>(
			"--reflang-include",
			"Complete #include line for reflang for generated code.",
			R"(#include "reflang.hpp")");
	auto out_hpp = cmd_args.Register<string>(
			"--out-hpp",
			"Output file path to write declarations (header) to. If empty "
			"stdout is used (outputs to console).",
			"");
	auto out_cpp = cmd_args.Register<string>(
			"--out-cpp",
			"Output file path to write definitions to. If empty, --out-hpp is "
			"used.",
			"");

	vector<string> files = GetFilesToProcess(cmd_args, argc, argv);
	Args args;
	while (--argc >= 0) args.push_back(*argv++);

	if (log_level->Get() >= 0) {
		gLogLevel = (LOG_LEVEL::LOG_LEVEL)log_level->Get();
#ifndef USE_RUNTIME_LOG_LEVEL
		log_warn << "command line option \"--log-level\" will be ignored because compile-time option USE_RUNTIME_LOG_LEVEL was not enabled";
#endif
	}

	::Options options;
	options.include = "^(" + filter_include->Get() + ")$";
	options.exclude = "^(" + filter_exclude->Get() + ")$";
	options.path_filter = path_filter->Get();

	options.include_path = reflang_include->Get();

	options.out_hpp_path = out_hpp->Get();
	ostream* out_decl = &cout;
	unique_ptr<ofstream> h_stream;
	if (!options.out_hpp_path.empty()) {
		h_stream = make_unique<ofstream>(options.out_hpp_path);
		out_decl = h_stream.get();
	}

	options.out_cpp_path = out_cpp->Get();
	ostream* out_impl = &cout;
	unique_ptr<ofstream> c_stream;
	if (!options.out_cpp_path.empty()) {
		c_stream = make_unique<ofstream>(options.out_cpp_path);
		out_impl = c_stream.get();
	}

	parseTypes(files, args, options);

	return 0;
}

