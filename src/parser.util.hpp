#ifndef REFLANG_PARSER_UTIL_HPP
#define REFLANG_PARSER_UTIL_HPP

#include <string>

#include <clang-c/Index.h>

namespace reflang
{
	namespace parser
	{
		std::string convert(const CXString &s);

		std::string getFullName(CXCursor cursor);
		std::string getName(const CXType &type);
		std::string getFile(const CXCursor &cursor);

		bool isRecursivelyPublic(CXCursor cursor);
	}
}

#endif //REFLANG_PARSER_UTIL_HPP

