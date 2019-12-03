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
//	{
//		*static_cast<CXString*>(this) = s;
//		s.release();
//	}
	XString& operator=(const XString& other) = delete;
	XString& operator=(XString&& other) = delete;

	XString(const CXString& s) { *static_cast<CXString*>(this) = s; }

	~XString() { if (data) clang_disposeString(*this); }

	explicit XString(const CXCursor&);
	explicit XString(const CXType&);
	explicit XString(CXCursorKind);
	explicit XString(CXTypeKind);

	operator std::string() const { if (auto s = clang_getCString(*this)) return s; else return ""; }

	bool empty() const { auto s = clang_getCString(*this); return !(s && *s); }
private:
	void release() { data = nullptr; }
};


inline auto& operator<<(std::ostream& os, const XString& s) { return os << std::string(s); }
std::ostream& operator<<(std::ostream& os, const CXType& t);

std::string buildFullName(CXCursor cursor);
std::string getFile(const CXCursor& cursor);

bool isOperatorFunction(CXCursor cursor);

#endif //KTN_CPP_UTILS_H
