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
	os << f.returnType.asCType() << " ";
	os << f.asCName() << "(";
	auto nArgs = f.arguments.size();

	if (f.receiver) {
		// hidden arg
		os << f.receiver->asCType();
		if (f.receiver->isConst()) os << " const";
		os << "* self";
		if (nArgs) os << ", ";
	}

	for (const auto& a : f.arguments) {
		os << a.asCType() << " " << a.name;
		if (--nArgs > 0) os << ", ";
	}
	os << ") {\n";
	os << "    return ";

	if (f.receiver) {
		// hidden arg
		os << "((" << f.receiver->asCxxType();
		if (f.receiver->isConst()) os << " const";
		os << "*)self)->";
	}

	os << f.getName() << "(";
	auto n = f.arguments.size(); // Awfull. I wish I had python-like join
	for (auto& a : f.arguments) {
		if (a.isRef()) os << "*";  // reference parameter was sent as pointer is C wrapper
		os << a.name;
		if (--n) os << ", ";
	}
	os << ");\n}";

	return os;
}

ostream& gen::genCxxDefinition(ostream& os, const Class& c)
{
//	TraceX(c.getFile());
	os << "// @class " << c.getFullName() << ":\n";
	for (const Function& f : c.methods) {
		if (!f.isInstanceMember()) os << "/*static*/ ";
		genCxxDefinition(os, f);
		os << '\n';
	}
	return os;
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
