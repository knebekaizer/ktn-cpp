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

bool ktn::WrapperGenerator::genCDeclaration(const Function& f, bool toDeclStream)
{
	// TODO move this to static member of class Generator
	static unordered_set<string> all_stubs;

	ostream& os = toDeclStream ? decl_ : impl_;
	bool rc = isTypeSupported(f);

	if (!rc) {
		os << "// [NOT SUPPORTED] ";
	}

	// don't declare stubs for unsupported functions
	if (rc && toDeclStream) {
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


bool ktn::WrapperGenerator::genCxxDefinition(const Function& f)
{
	if (!genCDeclaration(f, false)) {
		return false;
	}

	impl_ << " {\n";
	impl_ << "    ";

	if (!f.returnType.isVoid()) {
		impl_ << "return ";

		// Return type cast to C may be skipped if C and C++ return types are the same
		// or if ret type is pointer or reference (because all them are represented as void* in C and therefore does not require cast
		if (!f.returnType.isPtr() && !f.returnType.isRef() && f.returnType.isMangled()) {
			// I want to return non-trivial (ie mangled) type _by value_
			// C-type cast may be not possible, let's try to cast pointers instead: *(cType*)&
			impl_ << "*(" << f.returnType.asCType() << "*)& ";
			//TODO primitive typedefed types will be mangled but does not require cast at all
		}

		if (f.returnType.isRef())  impl_ << "&";  // return reference as pointer:
	}

	if (f.receiver) {
		// hidden arg
		impl_ << "((" << f.receiver->asCxxType();
		if (f.isConstMember()) impl_ << " const";
		impl_ << "*)self)->";
		impl_ << f.shortName();
	} else {
		impl_ << f.fullName();
	}

	impl_  << "(";
	auto n = f.arguments.size();
	for (auto& a : f.arguments) {
		if (a.isRef()) impl_ << " * ";  // reference parameter was sent as pointer is C wrapper

		if (a.isMangled()) {
			if (a.isRef()) {
				impl_ << "(" << a.pointee() << " * ) ";
			} else if (a.isPtr()) {  // prefer to use actual C++ type as declared in the method's signature
				impl_ << "(" << a.asCxxType() << ") ";  // however Pointee* is also valid
			} else {
				impl_ << "* (" << a.asCxxType() << " * ) &";
			}
		}

		impl_ << a.name;
		if (--n) impl_ << ", ";    // I wish I had python-like join
	}
	impl_ << ");\n}";

	return impl_ ? true : false;
}

bool ktn::WrapperGenerator::genWrapper(const Function& f)
{
	auto ok = genCDeclaration(f);
	decl_ << ";\n";

	if (ok) {
		genCxxDefinition(f) && impl_ << "\n";
	}
	return ok;
}

ostream& ktn::WrapperGenerator::genCxxDefinition(const Class& c)
{
	impl_ << "// @class " << c.fullName() << ":\n";
	for (const Function& f : c.methods) {
		if (!f.isInstanceMember()) impl_ << "/*static*/ ";
		genWrapper(f);
		impl_ << '\n';
	}
	return impl_;
}

void ktn::WrapperGenerator::genWrappers(Types::const_iterator begin, Types::const_iterator end)
{
	for (auto it = begin; it != end; ++it) {
		switch ((*it)->getType())
		{
			case TypeBase::Type::Enum:
				//	SerializeEnumHeader(*out_hpp, static_cast<const Enum&>(*type));
				break;
			case TypeBase::Type::Function:
				genWrapper(static_cast<const Function&>(**it));
				break;
			case TypeBase::Type::Class:
				genCxxDefinition(static_cast<const Class&>(**it)) && impl_ << "\n";
				break;
		}
	}

}
