#ifndef REFLANG_PARSER_ENUM_HPP
#define REFLANG_PARSER_ENUM_HPP

#include <clang-c/Index.h>

#include "types.hpp"

namespace reflang
{
	namespace parser
	{
		Enum getEnum(CXCursor cursor);
	}
}

#endif //REFLANG_PARSER_ENUM_HPP
