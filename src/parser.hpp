#ifndef REFLANG_PARSER_HPP
#define REFLANG_PARSER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>

#include "types.hpp"



class Parser {
public:

private:
	using Headers = std::unordered_set<std::string>;
	Headers files_;
};


#endif //REFLANG_PARSER_HPP
