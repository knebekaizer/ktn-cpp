#ifndef REFLANG_PARSER_ENUM_HPP
#define REFLANG_PARSER_ENUM_HPP

#include <clang-c/Index.h>

#include "types.hpp"

namespace ktn {

Enum buildEnum(CXCursor cursor);

}

#endif //REFLANG_PARSER_ENUM_HPP
