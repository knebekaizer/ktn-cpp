#include "types.hpp"
using namespace reflang;
using namespace std;

TypeBase::TypeBase(string file, string full_name)
:	full_name_(move(full_name))
,	file_(move(file))
{
}

TypeBase::~TypeBase() = default;

const string& TypeBase::getFullName() const
{
	return full_name_;
}

const string& TypeBase::getName() const
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

Function::Function(std::string file, std::string full_name)
:	TypeBase(move(file), move(full_name))
{
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
