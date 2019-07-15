#ifndef REFLANG_PARSER_FUNCTION_HPP
#define REFLANG_PARSER_FUNCTION_HPP

#include <clang-c/Index.h>

#include "types.hpp"

namespace reflang
{
	namespace parser
	{
		Function buildFunction(CXCursor cursor, const Class* receiver = nullptr);
	}
}

#endif //REFLANG_PARSER_FUNCTION_HPP
