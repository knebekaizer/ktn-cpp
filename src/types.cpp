#include "types.hpp"

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
	mangling_ = ensureUniqName(fullName());
}

std::string CxxType::asCType() const {
	return !ctype_name_.empty() ? ctype_name_ : type_name_ ;
}
