//
// Created by Vladimir Ivanov on 2019-07-15.
//

#include "generator.h"
#include "types.hpp"
#include "parser.util.hpp"

#include <ostream>
#include <string>

#include "trace.h"

using namespace std;
using namespace ktn;
using namespace generator;
namespace gen = generator;


ostream& gen::genCxxDefinition(ostream& os, const Function& f)
{
//	TraceX(f.returnType.kind());
	os << f.returnType.asCType() << " ";
	os << f.asCName() << "(";
	auto nArgs = f.arguments.size();

	if (f.receiver) {
		// hidden arg
	//	os << f.receiver->asCType();
	//	if (f.isConstMember()) os << " const";
	//	os << "* self";
	// KISS: void* instead of mangled type
		os << "void* self";
		if (nArgs) os << ", ";
	}

	for (const auto& a : f.arguments) {
		os << a.asCType() << " " << a.name;
		if (--nArgs > 0) os << ", ";
	}
	os << ") {\n";
	os << "    ";

	if (!f.returnType.isVoid()) {
		os << "return ";

		// Return type cast to C may be skipped if C and C++ return types are the same
		// or if ret type is pointer or reference (because all them are represented as void* in C and therefore does not require cast
		if (!f.returnType.isPtr() && !f.returnType.isRef() && f.returnType.isMangled()) {
			// I want to return non-trivial (ie mangled) type _by value_
			// C-type cast may be not possible, let's try to cast pointers instead: *(cType*)&
			os << "*(" << f.returnType.asCType() << "*)& ";
			//TODO primitive typedefed types will be mangled but does not require cast at all
		}

		if (f.returnType.isRef())  os << "&";  // return reference as pointer:
	}

	if (f.receiver) {
		// hidden arg
		os << "((" << f.receiver->asCxxType();
		if (f.isConstMember()) os << " const";
		os << "*)self)->";
		os << f.shortName();
	} else {
		os << f.fullName();
	}

	os  << "(";
	auto n = f.arguments.size();
	for (auto& a : f.arguments) {
		if (a.isRef()) os << " * ";  // reference parameter was sent as pointer is C wrapper

		if (a.isMangled()) {
			if (a.isRef()) {
				os << "(" << a.pointee() << " * ) ";
			} else if (a.isPtr()) {  // prefer to use actual C++ type as declared in the method's signature
				os << "(" << a.asCxxType() << ") ";  // however Pointee* is also valid
			} else {
				os << "* (" << a.asCxxType() << " * ) &";
			}
		}

		os << a.name;
		if (--n) os << ", ";    // I wish I had python-like join
	}
	os << ");\n}";

	return os;
}

ostream& gen::genCxxDefinition(ostream& os, const Class& c)
{
//	TraceX(c.getFile());
	os << "// @class " << c.fullName() << ":\n";
	for (const Function& f : c.methods) {
		if (!f.isInstanceMember()) os << "/*static*/ ";
		genCxxDefinition(os, f);
		os << '\n';
	}
	return os;
}

namespace {

bool isTypeSupported(const Function& f)  {
	// reject non-POD sent by value
	if (f.returnType.kind() == CxxType::KIND::OTHER)
		return false;

	for (auto& a : f.arguments) {
		if (a.kind() == CxxType::KIND::OTHER) {
			return false;
		}
	}
	return true;
};

}

void gen::genCxxDefinition(std::ostream& os, Types::const_iterator begin, Types::const_iterator end)
{
	for (auto it = begin; it != end; ++it) {
		switch ((*it)->getType())
		{
			case TypeBase::Type::Enum:
				//	SerializeEnumHeader(*out_hpp, static_cast<const Enum&>(*type));
				break;
			case TypeBase::Type::Function:
				// TODO: skip definition but print commented notice to declaration output
				if (!isTypeSupported(static_cast<const Function&>(**it))) {
					break;
				}
				genCxxDefinition(os, static_cast<const Function&>(**it));
				os << "\n";
				break;
			case TypeBase::Type::Class:
				genCxxDefinition(os, static_cast<const Class&>(**it));
				os << "\n";
				break;
		}
	}

}


void gen::genCxxDefinition(ostream& os, const std::vector<std::unique_ptr<TypeBase>>& types)
{
	for (const auto& type : types)
	{
		switch (type->getType())
		{
			case TypeBase::Type::Enum:
			//	SerializeEnumHeader(*out_hpp, static_cast<const Enum&>(*type));
				break;
			case TypeBase::Type::Function:
				genCxxDefinition(os, static_cast<const Function&>(*type));
				os << "\n";
				break;
			case TypeBase::Type::Class:
				genCxxDefinition(os, static_cast<const Class&>(*type));
				os << "\n";
				break;
		}
	}
}
