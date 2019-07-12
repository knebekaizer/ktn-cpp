#include "parser.util.hpp"

using namespace std;
using namespace reflang;

string parser::convert(const CXString &s) {
	string result = clang_getCString(s);
	clang_disposeString(s);
	return result;
}

std::ostream &operator<<(std::ostream &os, CXString &&s) {
	os << clang_getCString(s);
	clang_disposeString(s);
	return os;
}

std::ostream &operator<<(std::ostream &os, const CXType& t) {
	os << clang_getTypeSpelling(t) << "." << clang_getTypeKindSpelling(t.kind);
	return os;
}


string parser::getFullName(CXCursor cursor) {
	string name;
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		string cur = convert(clang_getCursorSpelling(cursor));
		if (name.empty()) {
			name = cur;
		} else {
			name = cur + "::" + name;
		}
		cursor = clang_getCursorSemanticParent(cursor);
	}

	return name;
}

string parser::getName(const CXType &type) {
	//TODO: unfortunately, this isn't good enough. It only works as long as the
	// type is fully qualified.
	return convert(clang_getTypeSpelling(type));
}

string parser::getFile(const CXCursor &cursor) {
	auto location = clang_getCursorLocation(cursor);
	CXFile file;
	clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);
	return convert(clang_getFileName(file));
}

bool parser::isRecursivelyPublic(CXCursor cursor) {
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		auto access = clang_getCXXAccessSpecifier(cursor);
		if (access == CX_CXXPrivate || access == CX_CXXProtected) {
			return false;
		}

		if (clang_getCursorLinkage(cursor) == CXLinkage_Internal) {
			return false;
		}

		if (clang_getCursorKind(cursor) == CXCursor_Namespace
		    && convert(clang_getCursorSpelling(cursor)).empty()) {
			// Anonymous namespace.
			return false;
		}

		cursor = clang_getCursorSemanticParent(cursor);
	}

	return true;
}
