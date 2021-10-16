//
// Created by Володя on 2019-08-21.
//

#ifndef KTN_CPP_UTILS_H
#define KTN_CPP_UTILS_H

#include "clang-c/Index.h"

#include <iostream>
#include <regex>


class XString : CXString {
public:
	XString(const XString&) = delete;
	XString(XString&& s) = delete;

	XString& operator=(const XString& other) = delete;
	XString& operator=(XString&& other) = delete;

	XString(const CXString& s) : CXString(s) {}

	~XString() { clang_disposeString(*this); }

	explicit XString(const CXCursor&);
	explicit XString(const CXType&);
	explicit XString(CXCursorKind);
	explicit XString(CXTypeKind);

	// not_null
	const char* cstr() const {
		if (!cstr_) {
			cstr_ = clang_getCString(*this);
			if (!cstr_) cstr_ = "";
		}
		return cstr_;
	}

	operator std::string() const { return cstr(); }

	bool empty() const { return !cstr()[0]; }
private:
	mutable const char* cstr_ = nullptr;
};


inline auto& operator<<(std::ostream& os, const XString& s) { return os << s.cstr(); }
std::ostream& operator<<(std::ostream& os, const CXType& t);

std::string buildFullName(CXCursor cursor);
std::string getContainingFile(const CXCursor& cursor);

bool isOperatorFunction(CXCursor cursor);

#endif //KTN_CPP_UTILS_H
