#include "parser.util.hpp"

using namespace std;

string ktn::convertAndDispose(const CXString &s) {
	auto cstr = clang_getCString(s);
	string result = cstr ? cstr : "";
	clang_disposeString(s);
	return result;
}

std::ostream &operator<<(std::ostream &os, CXString &&s) {
	auto cstr = clang_getCString(s);
	os << (cstr ? cstr : "");
	clang_disposeString(s);
	return os;
}

std::ostream &operator<<(std::ostream &os, const CXType& t) {
	os << clang_getTypeSpelling(t) << "." << clang_getTypeKindSpelling(t.kind);
	return os;
}


string ktn::getFullName(CXCursor cursor) {
	string name;
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		string cur = convertAndDispose(clang_getCursorSpelling(cursor));
		if (name.empty()) {
			name = cur;
		} else {
			name = cur + "::" + name;
		}
		cursor = clang_getCursorSemanticParent(cursor);
	}

	return name;
}

string ktn::getName(const CXType &type) {
	//TODO: unfortunately, this isn't good enough. It only works as long as the
	// type is fully qualified.
	return convertAndDispose(clang_getTypeSpelling(type));
}

string ktn::getFile(const CXCursor &cursor) {
	auto location = clang_getCursorLocation(cursor);
	CXFile file;
	clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);
	return convertAndDispose(clang_getFileName(file));
}

bool ktn::isRecursivelyPublic(CXCursor cursor) {
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		auto access = clang_getCXXAccessSpecifier(cursor);
		if (access == CX_CXXPrivate || access == CX_CXXProtected) {
			return false;
		}

		if (clang_getCursorLinkage(cursor) == CXLinkage_Internal) {
			return false;
		}

		if (clang_getCursorKind(cursor) == CXCursor_Namespace
		    && convertAndDispose(clang_getCursorSpelling(cursor)).empty()) {
			// Anonymous namespace.
			return false;
		}

		cursor = clang_getCursorSemanticParent(cursor);
	}

	return true;
}

bool ktn::isReference(const CXType& type)
{
	return type.kind == CXType_LValueReference || type.kind == CXType_RValueReference;
}

namespace {
int wildcmp(const char* wild, const char* string)
{
	if (!wild || !string) return -1;

	const char* cp = 0, * mp = 0;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string + 1;
		} else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	return !*wild;
}
}

bool ktn::WildCard::match(const std::string& s) const {
	return (bool)wildcmp(wildcard_.c_str(), s.c_str());
}

