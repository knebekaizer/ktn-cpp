//
// Created by Vladimir Ivanov on 2019-07-15.
//

#include "generator.h"
#include "types.hpp"

#include <ostream>
#include <string>

#include "trace.h"

using namespace std;
using namespace reflang;
namespace gen = generator;


ostream& gen::genCxxDefinition(ostream& os, const Function& f)
{
	TraceX(f);
	os << __PRETTY_FUNCTION__ << endl;

	os << f.returnType.asCType() << " ";
	os << f.asCName() << "(";
	auto nArgs = f.arguments.size();
/*
	if (f.memberOf) {
		// hidden arg
		os << f.memberOf->asCName();
		if (f.isConst) os << " const";
		os << "* self";
		if (nArgs) os << ", ";
	}
*/
	for (const auto& a : f.arguments) {
		os << a.asCType() << " " << a.name;
		if (--nArgs > 0) os << ", ";
	}
	os << ") {\n";
	os << "    return ";
/*
	if (f.memberOf) {
		// hidden arg
		os << "((" << f.memberOf->getFullName();
		if (f.isConst) os << " const";
		os << "*)self)->" << f.getName();
	}
*/
	os << "(";
	auto n = f.arguments.size(); // Awfull. I wish I had python-like join
	for (auto& a : f.arguments) {
		os << a.name;
		if (--n) os << ", ";
	}
	os << ");\n}\n";

	return os;
}

ostream& gen::genCxxDefinition(ostream& os, const Class& c) {
	TraceX(c.getFullName());
	for (const Function& f : c.methods) {
		genCxxDefinition(os, f);
		os << '\n';
	}
	return os;
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
				break;
			case TypeBase::Type::Class:
				genCxxDefinition(os, static_cast<const Class&>(*type));
				break;
		}
		os << "\n\n";
	}

}
