#include "serializer.function.hpp"

#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>

#include "serializer.util.hpp"

using namespace std;
using namespace reflang;

string serializer::mangling(string s) {
	replace(s.begin(), s.end(), ':', '_');
	return s;
}

ostream& serializer::genDefinition(ostream& os, const Function& f)
{
	os << mangling(f.returnType) << " ";
	os << mangling(f.getFullName()) << "(";
	for (auto k = 0; k != f.arguments.size(); ++k) {
		os << mangling(f.arguments[k].type) << " " << f.arguments[k].name;
		if (k < f.arguments.size() - 1) os << ", ";
	}
	os << ") {\n";
	os << "return " << f.getFullName() << "(";
	for (auto k = 0; k != f.arguments.size(); ++k) {
		os << f.arguments[k].type << " " << f.arguments[k].name;
		if (k < f.arguments.size() - 1) os << ", ";
	}
	os << ");\n}\n";
	return os;
}

namespace
{
	string CallFunction(const Function& f)
	{
		stringstream s;
		s << f.getFullName() << "(";
		int i = 0;
		for (const auto& arg : f.arguments)
		{
			s << "args[" << i << "].GetT<std::decay_t<" << arg.type << ">>()";
			if (i != f.arguments.size() - 1)
			{
				s << ", ";
			}
			++i;
		}
		s << ")";
		return s.str();
	}

	// Returns a different string for equal names. This is useful for overload
	// disambiguation. For example, first time called with "foo" it will return
	// "", second time will return "_1", third time will return "_2", etc.
	string GetUniqueSuffixForString(const string& name)
	{
		static unordered_map<string, int> unique_ids;
		int current_id = unique_ids[name]++;
		string unique_id = "_" + to_string(current_id);
		if (unique_id == "_0")
		{
			// No need to make the generated code ugly for the first instance of
			// the function (which probably has no overloads anyway).
			unique_id.clear();
		}
		return unique_id;
	}
}

string serializer::GetFunctionSignature(const Function& f)
{
	stringstream s;
	s << f.returnType << "(*)(";
	for (size_t i = 0; i < f.arguments.size(); ++i)
	{
		s << f.arguments[i].type;
		if (i != f.arguments.size() - 1)
		{
			s << ", ";
		}
	}
	s << ")";
	return s.str();
}

void serializer::SerializeFunctionHeader(ostream& o, const Function& f)
{
	stringstream tmpl;
	tmpl << R"(
template <>
class Function<%signature%, %name%> : public IFunction
{
	int GetParameterCount() const override;
	Parameter GetReturnType() const override;
	Parameter GetParameter(int i) const override;

	const std::string& getName() const override;

	Object Invoke(const std::vector<Object>& args) override;
};
)";

	o << replaceAll(
			tmpl.str(),
			{
					{"%name%",      f.getFullName()},
					{"%signature%", serializer::GetFunctionSignature(f)}
			});
}

void serializer::SerializeFunctionSources(ostream& o, const Function& f)
{
	stringstream tmpl;
	tmpl << R"(
int Function<%signature%, %name%>::GetParameterCount() const
{
	return %arg_count%;
}

Parameter Function<%signature%, %name%>::GetReturnType() const
{
	Parameter result;
	result.type = "%return_type%";
	return result;
}

Parameter Function<%signature%, %name%>::GetParameter(int i) const
{
	if (i < 0 || i >= GetParameterCount())
	{
		throw Exception("Argument out of range.");
	}

	Parameter result;
)";
	if (!f.arguments.empty())
	{
		tmpl << R"(
	switch (i)
	{
)";
		for (size_t i = 0; i < f.arguments.size(); ++i)
		{
			tmpl << "	case " << i << ":\n";
			tmpl << "		result.name = \"" << f.arguments[i].name << "\";\n";
			tmpl << "		result.type = \"" << f.arguments[i].type << "\";\n";
			tmpl << "		break;\n";
		}
		tmpl << R"(	default:
		break;
	}

)";
	}
	tmpl << R"(	return result;
}

static const std::string %escaped_name%%unique_id%_name = "%name%";

const std::string& Function<%signature%, %name%>::getName() const
{
	return %escaped_name%%unique_id%_name;
}

Object Function<%signature%, %name%>::Invoke(const std::vector<Object>& args)
{
	if (args.size() != GetParameterCount())
	{
		throw Exception("Invoke(): bad argument count.");
	}
)";

	if (f.returnType == "void")
	{
		tmpl << R"(
	%call_function%;
	return Object();)";
	}
	else
	{
		tmpl << R"(
	return Object(%call_function%);)";
	}
	tmpl << R"(
}

namespace
{
	// Object to auto-register %name%.
	struct %escaped_name%%unique_id%_registrar
	{
		%escaped_name%%unique_id%_registrar()
		{
			::reflang::registry::internal::Register(
					std::make_unique<Function<%signature%, %name%>>());
		}
	} %escaped_name%%unique_id%_instance;
}
)";

	o << replaceAll(
			tmpl.str(),
			{
					{"%name%",          f.getFullName()},
					{"%signature%",     GetFunctionSignature(f)},
					{"%arg_count%",     to_string(f.arguments.size())},
					{"%return_type%",   f.returnType},
					{"%call_function%", CallFunction(f)},
					{"%escaped_name%",  getNameWithoutColons(f.getFullName())},
					{"%unique_id%",     GetUniqueSuffixForString(f.getFullName())}
			});
}
