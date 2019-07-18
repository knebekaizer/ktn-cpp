#include <iostream>
#include <fstream>
#include <string>

#include "cmdargs.hpp"
#include "parser.hpp"
#include "parser.util.hpp"
#include "generator.h"

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

LOG_LEVEL::LOG_LEVEL gLogLevel = DEF_LOG_LEVEL;


int main(int argc, char **argv)
{
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

	if (log_level->Get() >= 0) {
		gLogLevel = (LOG_LEVEL::LOG_LEVEL)log_level->Get();
#ifndef USE_RUNTIME_LOG_LEVEL
		log_warn << "command line option \"--log-level\" will be ignored because compile-time option USE_RUNTIME_LOG_LEVEL was not enabled";
#endif
	}

	ktn::Options options;
	options.include = "^(" + filter_include->Get() + ")$";
	options.exclude = "^(" + filter_exclude->Get() + ")$";
	options.path_filter = path_filter->Get();

	if (list_only->Get()) {
		auto names = ktn::getSupportedTypeNames(files, argc, argv, options);
		for (const auto& it : names) {
			cout << it << endl;
		}
		return 0;
	}

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
	ktn::WrapperGenerator gen(*out_decl, *out_impl);

	auto types = ktn::getTypes(files, argc, argv, options, &gen);

	return 0;
}

