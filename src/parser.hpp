#ifndef REFLANG_PARSER_HPP
#define REFLANG_PARSER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>

#include "types.hpp"

namespace ktn {

class WrapperGenerator;

struct Options {
	std::regex include;
	std::regex exclude;
	std::string path_filter;  // wildcard pattern

	std::string include_path;
	std::string out_hpp_path;
	std::string out_cpp_path;
	//TODO: bool standalone = false;
};

std::vector<std::string> getSupportedTypeNames(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options = Options());

std::vector<std::unique_ptr<TypeBase>> getTypes(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options = Options(),
		WrapperGenerator* generator = nullptr);

class Parser {
public:

private:
	using Headers = std::unordered_set<std::string>;
	Headers files_;
};

}

#endif //REFLANG_PARSER_HPP
