//
// Created by Vladimir Ivanov on 06.11.2021.
//

#ifndef KTN_CPP_XCLANG_HPP
#define KTN_CPP_XCLANG_HPP

#include <clang-c/Index.h>
#include "utils.h"

struct Entity {
	std::string			usr;
	std::string			name;
	CXCursorKind	cursorKind = (CXCursorKind)0;
	CXTypeKind		typeKind = CXType_Invalid;
	std::string			typeName;

	std::string cursorKindS() const { return str(cursorKind); }
	std::string typeKindS() const { return str(typeKind); }
};

struct XType : CXType {
	XType(const CXType& t) : CXType(t) {}	 // NOLINT(google-explicit-constructor)
	std::string name() const { return XString(*this); }
	std::string kindS() const { return XString(kind); }
};

Entity serialize(struct XCursor const& c);

struct XCursor : CXCursor {
	XCursor(const CXCursor& c) : CXCursor(c) {}	   // NOLINT(google-explicit-constructor)

	std::string spelling() const { return XString(*this); }
	std::string name() const { return XString(*this); }
	auto kind() const { return clang_getCursorKind(*this); }
	XType type() const { return clang_getCursorType(*this); }
	std::string typeName() const { return XString(type()); }
	std::string usr() const { return XString(clang_getCursorUSR(*this)); }

	// TODO remove from members
	Entity data() const { return serialize(*this); }

	std::string kindS() const { return str(kind()); }
	std::string typeKindS() const { return type().kindS(); }
};

#endif //KTN_CPP_XCLANG_HPP
