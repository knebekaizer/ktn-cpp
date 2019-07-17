#include "parser.util.hpp"
#include "types.hpp"

#include <clang-c/Index.h>


using namespace std;
using namespace ktn;

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


string ktn::buildFullName(CXCursor cursor) {
	string name;
	while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
		string cur = convertAndDispose(clang_getCursorSpelling(cursor));
		if (name.empty()) {
			name = cur;
		} else {
			name.insert(0, cur + "::");
		}
		cursor = clang_getCursorSemanticParent(cursor);
	}

	return name;
}

string ktn::getTypeSpelling(const CXType& type) {
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

bool ktn::isRefType(const CXType& type)
{
	return type.kind == CXType_LValueReference || type.kind == CXType_RValueReference;
}


/*
 * Rationale: why not CxxType(CxType) constructor?
 * - Constructor is merely to _construct_ object from its details
 * - Here we need a _conversion_ from one domain (clang) to another (KTN)
 * - For the sake of _low coupling_ it's better to keep domains isolated, while this only function depends on both
 * (is it Dependency Inversion?)
 */
CxxType ktn::buildCxxType(CXType type) {
	auto canonical = clang_getCanonicalType(type);
	auto is_ref = isRefType(canonical.kind == CXType_Invalid ? type : canonical);
	return CxxType(getTypeSpelling(type), is_ref);
}

CxxType ktn::buildCxxType(CXCursor cursor) {
	assert(clang_isDeclaration(clang_getCursorKind(cursor))); // caller is responsible to use in the proper context only!
	return buildCxxType(clang_getCursorType(cursor));
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

