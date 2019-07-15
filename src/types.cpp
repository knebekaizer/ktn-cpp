#include "types.hpp"
#include "serializer.util.hpp"

#include <unordered_set>

#include "trace.h"

using namespace reflang;
using namespace std;


// +++ Mangling ++++++++++++++++++++++++++++++++++++++++++++++++++++=
namespace {

string simpleMangling(string s) {
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
	: full_name_(move(full_name))
	, file_(move(file))
	, mangling_(simpleMangling(full_name_))
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


Function::Function(std::string file, std::string full_name)
		:	TypeBase(move(file), move(full_name))
{
	mangling_ = ensureUniqName(mangling_);
}

std::string CxxType::asCType() const {
	// quick-n-dirty mangling, may not work
	return simpleMangling(type);
}


// Move to serializer namespace
ostream& genDefinition(ostream& os, const Function& f)
{
	os << f.returnType.asCType() << " ";

	os << f.asCName() << "(";
	for (auto k = 0; k != f.arguments.size(); ++k) {
		os << f.arguments[k].asCType() << " " << f.arguments[k].name;
		if (k < f.arguments.size() - 1) os << ", ";
	}
	os << ") {\n";
	os << "return "
	   // Hidden arg:
	   //      ((classFullNameWithConstPtr)self)->shortName   // member function
	   //      fullName                                       // static member or global
	   << f.getFullName() << "(";
	auto n = f.arguments.size(); // Awfull. I wish I had python-like join
	for (auto& a : f.arguments) {
		// Optional cast to CxxType
		// if (a.isCastNeeded()) os << "(" << a.asCxxType() << ")"
		// if LValueReferenceType: os << "*"; // demangling: lvref represented as pointer in C and as value in C++
		os << a.name;
		if (--n) os << ", ";
	}
	os << ");\n}\n";
	return os;
}
