#include "parser.util.hpp"
#include "types.hpp"

#include <clang-c/Index.h>

#include <string>
#include <set>
#include <regex>


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
	Trace2( clang_getCursorType(cursor), name );

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

namespace {

/*
bool isPrimType_(CXType type) {
	static const set<CXTypeKind> prim_types = {
			CXType_Void, // = 2,
			CXType_Bool, // = 3,
			CXType_Char_U, // = 4,
			CXType_UChar, // = 5,
			CXType_Char16, // = 6,
			CXType_Char32, // = 7,
			CXType_UShort, // = 8,
			CXType_UInt, // = 9,
			CXType_ULong, // = 10,
			CXType_ULongLong, // = 11,
			CXType_UInt128, // = 12,
			CXType_Char_S, // = 13,
			CXType_SChar, // = 14,
			CXType_WChar, // = 15,
			CXType_Short, // = 16,
			CXType_Int, // = 17,
			CXType_Long, // = 18,
			CXType_LongLong, // = 19,
			CXType_Int128, // = 20,
			CXType_Float, // = 21,
			CXType_Double, // = 22,
			CXType_LongDouble, // = 23,
	};
	return prim_types.find(type.kind) != prim_types.end();
}
*/

}

string ktn::simpleMangling(string s, const char* prefix) {
	// sort of uniq (uncommon) prefix

	if (prefix && *prefix) {
		// FIXME Dirty and fragile. Will be broken when Clang changes spelling format.
		regex pattern("^(const +)*");
		s = regex_replace(s, pattern, string("$1") + prefix);
	}

	replace(s.begin(), s.end(), ':', '_');
	replace(s.begin(), s.end(), '&', '*');
	return s;
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
	auto real_type = canonical.kind == CXType_Invalid ? type : canonical;
	CxxType::KIND kind = clang_isPODType(real_type) ? kind = CxxType::KIND::POD : CxxType::KIND::OTHER;
	switch (real_type.kind) {
		case CXType_Invalid:
			kind = CxxType::KIND::INVALID; break;
		case CXType_Void:
			kind = CxxType::KIND::VOID; break;
		case CXType_Pointer:
			kind = CxxType::KIND::PTR; break;
		case CXType_LValueReference:
		case CXType_RValueReference:  // ???
			kind = CxxType::KIND::REF; break;
		default:
			break;
	}

	bool is_const = clang_isConstQualifiedType(real_type);
//TraceX(real_type);
	auto name = getTypeSpelling(type);
	string cname;
	string pointee;

	switch (real_type.kind) {
		case CXType_Elaborated:
		case CXType_Record:
			cname = simpleMangling(name);
			break;
		case CXType_Pointer:
		case CXType_LValueReference:
		case CXType_RValueReference:  // ???
			cname = "void *"; // TODO shoud i use opaque C pointer type to get type checking at compile time?
//TraceX(clang_getPointeeType(real_type));
			pointee = getTypeSpelling(clang_getPointeeType(real_type));
			break;
		default:
			break;
	}
//Trace2(name, cname);
	return CxxType(name, kind, is_const, cname, pointee);
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

