#include "utils.h"

#include <clang-c/Index.h>

#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>

#include "trace.h"

using namespace std;

XString::XString(const CXCursor& x) : XString(clang_getCursorSpelling(x)) {}
XString::XString(const CXType& x) : XString(clang_getTypeSpelling(x)) {}
XString::XString(CXTypeKind x)  : XString(clang_getTypeKindSpelling(x)) {}
XString::XString(CXCursorKind x)  : XString(clang_getCursorKindSpelling(x)) {}


string simpleMangling(string s, const char* prefix) {
	// sort of uniq (uncommon) prefix

	if (prefix && *prefix) {
		// FIXME Dirty and fragile. Will be broken when Clang changes spelling format.
		regex pattern("^(const +)*");
		s = regex_replace(s, pattern, string("$1") + prefix);
	}

	replace(s.begin(), s.end(), ':', '_');
	replace(s.begin(), s.end(), '&', '*');

	// operators
	s = regex_replace(s, regex("<<"), "Left");
	s = regex_replace(s, regex(">>"), "Right");
	s = regex_replace(s, regex("\\+\\+"), "Incr");
	s = regex_replace(s, regex("--"), "Decr");
	s = regex_replace(s, regex("\\+="), "IncrBy");
	s = regex_replace(s, regex("-="), "DecrBy");

	s = regex_replace(s, regex("=="), "Equal");
	s = regex_replace(s, regex("<"), "Less");
	s = regex_replace(s, regex(">"), "More");
	s = regex_replace(s, regex("="), "Assign");

	return s;
}

std::ostream& operator<<(std::ostream &os, const CXType& t) {
	os << clang_getTypeSpelling(t) << "." << clang_getTypeKindSpelling(t.kind);
	return os;
}

//bool isOperatorFunction(CXCursor cursor) {
//	return convertAndDispose(clang_getCursorSpelling(cursor)).substr(0, 8) == "operator";
//}


string buildFullName(CXCursor cursor) {
	string name;
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		string cur = XString(cursor);
		if (name.empty()) {
			name = cur;
		} else {
			name.insert(0, cur + "::");
		}
		cursor = clang_getCursorSemanticParent(cursor);
	}
	Trace2( clang_getCursorType(cursor), name );

	return name;
}

string getFile(const CXCursor &cursor) {
	auto location = clang_getCursorLocation(cursor);
	CXFile file;
	clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);
	return XString(clang_getFileName(file));
}

bool isRecursivelyPublic(CXCursor cursor) {
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		auto access = clang_getCXXAccessSpecifier(cursor);
		if (access == CX_CXXPrivate || access == CX_CXXProtected) {
			return false;
		}

		if (clang_getCursorLinkage(cursor) == CXLinkage_Internal) {
			return false;
		}

		if (clang_getCursorKind(cursor) == CXCursor_Namespace
		    && XString(cursor).empty()) {
			// Anonymous namespace.
			return false;
		}

		cursor = clang_getCursorSemanticParent(cursor);
	}

	return true;
}

