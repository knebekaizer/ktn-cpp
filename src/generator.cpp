//
// Created by Vladimir Ivanov on 2019-07-15.
//

#include "generator.h"
#include "types.hpp"
#include "parser.util.hpp"

#include <ostream>
#include <string>
#include <unordered_set>

#include "trace.h"

using namespace std;
using namespace ktn;
using namespace generator;
namespace gen = generator;

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

bool ensureTypeDeclaration(std::ostream &os, const CxxType& t) {
	return false;
}

}

bool gen::genCDeclaration(std::ostream &os, const Function& f, bool doTypeStubs)
{
	// TODO move this to static member of class Generator
	unordered_set<string> all_stubs;

	bool rc = isTypeSupported(f);

	if (!rc) {
		os << "// [NOT SUPPORTED] ";
	}

	// don't declare stubs for unsupported functions
	if (rc && doTypeStubs) {
		ensureTypeDeclaration(os, f.returnType);
		for (auto& a : f.arguments) {
			ensureTypeDeclaration(os, a);
		}
	}

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
	os << ")";
	// No semicolon at the end!
	return rc;
}


bool gen::genCxxDefinition(ostream& os, const Function& f)
{
	if (!genCDeclaration(os, f)) {
		return false;
	}

	os << " {\n";
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

	return os ? true : false;
}

bool gen::genWrapper(std::ostream& out_decl, std::ostream& out_def, const Function& f)
{
	auto ok = genCDeclaration(out_decl, f);
	out_decl << ";\n";

	if (ok) {
		genCxxDefinition(out_def, f) && out_def << "\n";
	}
	return ok;
}

ostream& gen::genCxxDefinition(ostream& os, const Class& c)
{
	os << "// @class " << c.fullName() << ":\n";
	for (const Function& f : c.methods) {
		if (!f.isInstanceMember()) os << "/*static*/ ";
		genCxxDefinition(os, f);
		os << '\n';
	}
	return os;
}

void gen::genWrappers(std::ostream& out_decl, std::ostream& out_def, Types::const_iterator begin, Types::const_iterator end)
{
	for (auto it = begin; it != end; ++it) {
		switch ((*it)->getType())
		{
			case TypeBase::Type::Enum:
				//	SerializeEnumHeader(*out_hpp, static_cast<const Enum&>(*type));
				break;
			case TypeBase::Type::Function:
				genWrapper(out_decl, out_def, static_cast<const Function&>(**it));
				break;
			case TypeBase::Type::Class:
				genCxxDefinition(out_def, static_cast<const Class&>(**it)) && out_def << "\n";
				break;
		}
	}

}
