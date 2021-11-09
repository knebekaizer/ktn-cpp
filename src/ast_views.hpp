//
// Created by Vladimir Ivanov on 06.11.2021.
//

#ifndef KTN_CPP_AST_VIEWS_HPP
#define KTN_CPP_AST_VIEWS_HPP

#include <clang-c/Index.h>

#include "range/v3/all.hpp"
#include "xclang.hpp"
#include "utils.h"

namespace xclang::views {
inline // how to avoid declaring type?
	auto argumenttCursors(const XCursor& funC) {
//		if (auto kind = funC.kind(); kind != CXCursor_FunctionDecl && kind != CXCursor_CXXMethod) {
//			throw std::invalid_argument("Illegal CXCursor kind")
//		}
		auto numArgs = clang_Cursor_getNumArguments(funC);
		if (numArgs < 0) throw std::invalid_argument("Illegal CXCursor kind");
		return ranges::views::iota(0, numArgs) | ranges::views::transform(
			[&funC](auto i){ return (XCursor const&)clang_Cursor_getArgument(funC, i); }
		);
	}
}

#endif //KTN_CPP_AST_VIEWS_HPP
