#include "types.hpp"

#include <unordered_set>

#include "trace.h"

using namespace std;


// +++ Mangling ++++++++++++++++++++++++++++++++++++++++++++++++++++=
namespace {

string simpleMangling(string s) {
	constexpr char prefix[] = "$_";  // sort of uniq (uncommon) prefix
	s.insert(0, prefix);
	replace(s.begin(), s.end(), ':', '_');
	replace(s.begin(), s.end(), '&', '*');
	return s;
}

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
	, mangling_(simpleMangling(full_name_))
{
}

TypeBase::~TypeBase() = default;

const string& TypeBase::fullName() const
{
	return full_name_;
}

const string& TypeBase::getName() const
{
	return full_name_;
}

const string& TypeBase::asCName() const
{
	return mangling_;
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


Function::Function(std::string file, std::string full_name, bool constMember)
		:	TypeBase(move(file), move(full_name))
{
	mangling_ = ensureUniqName(mangling_);
}

std::string CxxType::asCType() const {
	// quick-n-dirty mangling, may not work
	return simpleMangling(type);
}
