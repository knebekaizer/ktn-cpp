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
	auto argumenttCursors(const XCursor& funcC) {
		auto numArgs = clang_Cursor_getNumArguments(funcC);
		if (numArgs < 0) throw std::runtime_error("Illegal CXCursor kind");
		return ranges::views::iota(0, numArgs) | ranges::views::transform(
			[&funcC](auto i){ return (XCursor const&)clang_Cursor_getArgument(funcC, i); }
		);
	}
}

#endif //KTN_CPP_AST_VIEWS_HPP
