#ifndef REFLANG_PARSER_UTIL_HPP
#define REFLANG_PARSER_UTIL_HPP

#include <string>

#include <clang-c/Index.h>
#include "trace.h"

namespace reflang
{
	namespace parser
	{
		std::string convertAndDispose(const CXString &s);

		std::string getFullName(CXCursor cursor);
		std::string getName(const CXType &type);
		std::string getFile(const CXCursor &cursor);

		bool isRecursivelyPublic(CXCursor cursor);
	}
}

std::ostream& operator<<(std::ostream& os, CXString &&s);
std::ostream &operator<<(std::ostream &os, const CXType& t);


#endif //REFLANG_PARSER_UTIL_HPP

