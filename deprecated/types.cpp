#include "types.hpp"
#include "parser.util.hpp"

#include <unordered_set>

#include "trace.h"

using namespace std;


// +++ Mangling ++++++++++++++++++++++++++++++++++++++++++++++++++++=
namespace {

const std::string ensureUniqName(std::string name) {
	// quick & dirty:
	static std::unordered_set<std::string> symbols; // TODO string is duplicated. Return ref / ptr to the element of the container
	int k = 0;
	auto pos = name.size();
	while (!symbols.insert(name).second) { // TODO move arg then emplace here
		name.replace(pos, name.size(), std::to_string(++k));
	}
	return name;
}

}


TypeBase::TypeBase(string file, string full_name)
	: file_(move(file))
	, full_name_(move(full_name))
{
}

TypeBase::~TypeBase() = default;

const string& TypeBase::fullName() const
{
	return full_name_;
}


const string& TypeBase::getFile() const
{
	return file_;
}

Enum::Enum(string file, string full_name)
:	TypeBase(move(file), move(full_name))
{
}

Enum::Type Enum::getType() const
{
	return Type::Enum;
}

Function::Type Function::getType() const
{
	return Type::Function;
}

Class::Class(string file, string full_name)
:	TypeBase(move(file), move(full_name))
{
}

Class::Type Class::getType() const
{
	return Type::Class;
}


Function::Function(std::string file, std::string full_name, bool const_)
		: TypeBase(move(file), move(full_name))
		, const_member_(const_)
{
	mangling_ = ensureUniqName(ktn::simpleMangling(fullName()));
}

std::string CxxType::asCType() const {
	return !ctype_name_.empty() ? ctype_name_ : type_name_ ;
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

	// Signed int type is used. Return value may be negative in case of error;
	// CxxType uses unsigned and considers 0 as invalid (erroneous) value.
	auto size = real_type.kind > CXType_Void ? clang_Type_getSizeOf(type) : 0;
	if (size < 0) {
		//	log_warn << "size error: " << size << " for type " << type;
		size = 0;
	}

	bool is_const = clang_isConstQualifiedType(real_type);
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
			pointee = getTypeSpelling(clang_getPointeeType(real_type));
			break;
		default:
			break;
	}
	return CxxType(name, size, kind, is_const, cname, pointee);
}

CxxType ktn::buildCxxType(CXCursor cursor) {
	assert(clang_isDeclaration(clang_getCursorKind(cursor))); // caller is responsible to use in the proper context only!
	return buildCxxType(clang_getCursorType(cursor));
}

