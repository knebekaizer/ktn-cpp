#ifndef REFLANG_PARSER_CLASS_HPP
#define REFLANG_PARSER_CLASS_HPP

#include <clang-c/Index.h>

#include "types.hpp"

namespace ktn {

Class buildClass(CXCursor cursor);

}

#endif //REFLANG_PARSER_CLASS_HPP