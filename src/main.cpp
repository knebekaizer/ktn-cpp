#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

//#include "cmdargs.hpp"

#include "parser_tree.h"

#include "trace.h"
#ifdef USE_RUNTIME_LOG_LEVEL
LOG_LEVEL::LOG_LEVEL gLogLevel = (LOG_LEVEL::LOG_LEVEL)DEF_LOG_LEVEL;
#endif

#define CXXOPTS_NO_RTTI
#include "cxxopts.hpp"

using namespace std;

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
					("I,include", "Include path", value<std::vector<std::string>>()->default_value({}), "PATH")
					("P,parse-options", "Compiler options to be directly sent to the clang parser", value<std::vector<std::string>>()->default_value({}))
					("l,log-level", "Log level, as digit or word [0:none, 1:fatal, 2:error, 3:warn, 4:info, 5:debug, 6:trace]", cxxopts::value<std::string>())
					("help", "Print help");

	options.parse_positional({"input"});

	auto opts = options.parse(argc, argv);

	if (opts.count("help")) {
		std::cout << options.help({"", "Group"}) << std::endl;
		exit(0);
	}

	if (opts.count("log-level")) {
#ifdef USE_RUNTIME_LOG_LEVEL
		std::vector<std::string> levels = {"0", "none", "1", "fatal", "2", "error", "3", "warn", "4", "info", "5", "debug", "6", "trace"};
		auto found = std::find(levels.begin(), levels.end(), opts["log-level"].as<std::string>());
		if (found != levels.end()) {
			unsigned int level = (found - levels.begin()) / 2;
			assert(level < LOG_LEVEL::LOG_LEVEL::invalid);
			gLogLevel = (LOG_LEVEL::LOG_LEVEL)level;
		}
#else
		log_warn << "command line option \"--log-level\" will be ignored because compile-time option USE_RUNTIME_LOG_LEVEL was not enabled";
#endif
	}
	log_info << "Log level is " << LOG_LEVEL_;

	files = opts["input"].as<std::vector<std::string>>();

//	parse_options = opts["parse-options"].as<std::vector<std::string>>();
	parse_options = {"-isystem"
			//	, "/Volumes/vdi/.konan/dependencies/clang-llvm-apple-8.0.0-darwin-macos/lib/clang/8.0.0/include"
				, "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1"
				, "-B/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin"
				, "-fno-stack-protector"
				, "--sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk"
				, "-mmacosx-version-min=10.11"
				, "-I/Library/Developer/CommandLineTools/usr/lib/clang/10.0.1/include/"
				};

	if (opts.count("include")) {
		auto const& r = opts["include"].as<std::vector<std::string>>();
		transform(r.begin(), r.end(), back_inserter(parse_options), [](auto s) { return "-I" + s;});
	}

	log_trace << "files:\n    " << joinToString(files, "\n    ");
	log_trace << "parse_options:\n    " << joinToString(parse_options, "\n    ");
}

int main(int argc, char **argv)
{
    TraceX(__clang_version__);

    try {
    	Config config(argc, argv);

	parseTypes(config.files, config.parse_options);

	return 0;

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
}

