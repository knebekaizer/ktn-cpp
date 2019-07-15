#include "serializer.class.hpp"

#include <map>
#include <sstream>
#include <vector>

#include "serializer.function.hpp"
#include "serializer.util.hpp"

using namespace std;
using namespace reflang;

namespace
{
	string IterateFields(const Class& c)
	{
		stringstream tmpl;

		for (const auto& field : c.fields)
		{
			tmpl << "	t(c." << field.name << ");\n";
		}

		return tmpl.str();
	}

	string IterateStaticFields(const Class& c)
	{
		stringstream tmpl;

		for (const auto& field : c.staticFields)
		{
			tmpl << "	t(" << c.getFullName() << "::" << field.name << ");\n";
		}

		return tmpl.str();
	}

	string MethodDeclaration(const Class& c, const Function& m)
	{
		stringstream tmpl;
		tmpl << R"(template <>
class Method<decltype(%name%), %name%> : public IMethod
{
public:
	const std::string& getName() const override;
	int GetParameterCount() const override;
	Object Invoke(const Reference& o, const std::vector<Object>& args) override;
};

)";
		return serializer::replaceAll(
				tmpl.str(),
				{
						{"%name%", "&" + c.getFullName() + "::" + m.name},
				});
	}

	string MethodsDeclarations(const Class& c)
	{
		if (c.methods.empty())
		{
			return string();
		}

		stringstream tmpl;
		tmpl << "// " << c.getFullName() << " methods metadata.\n";

		for (const auto& method : c.methods)
		{
			tmpl << MethodDeclaration(c, method);
		}

		tmpl << "// End of " << c.getFullName() << " methods metadata.\n";

		return tmpl.str();
	}

	string StaticMethodsDeclarations(const Class& c)
	{
		if (c.staticMethods.empty())
		{
			return string();
		}

		stringstream tmpl;
		tmpl << "// " << c.getFullName() << " static methods metadata.\n";

		for (const auto& method : c.staticMethods)
		{
			serializer::SerializeFunctionHeader(tmpl, method);
		}

		tmpl << "// End of " << c.getFullName() << " static methods metadata.\n";

		return tmpl.str();
	}

	string GetCallArgs(const Function& m)
	{
		stringstream tmpl;
		for (size_t i = 0; i < m.arguments.size(); ++i)
		{
			tmpl << "args[" << i << "].GetT<std::decay_t<"
				<< m.arguments[i].type << ">>()";
			if (i != m.arguments.size() - 1)
			{
				tmpl << ", ";
			}
		}
		return tmpl.str();
	}

	string MethodDefinition(const Class& c, const Function& m)
	{
		stringstream tmpl;
		tmpl << R"(static std::string %escaped_name%_name = "%name%";

const std::string& Method<decltype(%pointer%), %pointer%>::getName() const
{
	return %escaped_name%_name;
}

int Method<decltype(%pointer%), %pointer%>::GetParameterCount() const
{
	return %param_count%;
}

Object Method<decltype(%pointer%), %pointer%>::Invoke(
		const Reference& o, const std::vector<Object>& args)
{
	if (args.size() != %param_count%)
	{
		throw Exception("Invoke(): bad argument count.");
	}
)";
		if (m.returnType.type == "void")
		{
			tmpl << R"(	((o.GetT<%class_name%>()).*(%pointer%))(%call_args%);
	return Object();
)";
		}
		else
		{
			tmpl << R"(	return Object(((o.GetT<%class_name%>()).*(%pointer%))(%call_args%));
)";
		}
		tmpl << R"(}

)";
		return serializer::replaceAll(
				tmpl.str(),
				{
						{"%class_name%",  c.getFullName()},
						{"%pointer%",     "&" + c.getFullName() + "::" + m.name},
						{"%name%",        m.name},
						{
						 "%escaped_name%",
						                  serializer::getNameWithoutColons(
								                  c.getFullName()) + "_" + m.name
						},
						{"%param_count%", to_string(m.arguments.size())},
						{"%call_args%",   GetCallArgs(m)}
				});
	}

	string MethodsDefinitions(const Class& c)
	{
		if (c.methods.empty())
		{
			return string();
		}

		stringstream tmpl;
		tmpl << "// " << c.getFullName() << " methods definitions.\n";

		for (const auto& method : c.methods)
		{
			tmpl << MethodDefinition(c, method);
		}

		tmpl << "// End of " << c.getFullName() << " methods definitions.\n";

		return tmpl.str();
	}

	map<string, vector<Function>> GetMethodsByName(
			const Class::Methods& methods)
	{
		map<string, vector<Function>> methods_by_name;
		for (const auto& method : methods)
		{
			methods_by_name[method.name].push_back(method);
		}
		return methods_by_name;
	}

	string GetMethodImpl(const Class& c)
	{
		map<string, vector<Function>> methods_by_name = GetMethodsByName(
				c.methods);

		stringstream tmpl;
		bool first = true;
		for (const auto& methods : methods_by_name)
		{
			tmpl << "	";
			if (first)
			{
				first = false;
			}
			else
			{
				tmpl << "else ";
			}
			tmpl << "if (name == \"" << methods.first << "\")\n";
			tmpl << "	{\n";
			for (const auto& method : methods.second)
			{
				string name = "&" + c.getFullName() + "::" + methods.first;
				tmpl << "		results.push_back(std::make_unique<Method<decltype("
					<< name << "), " << name << ">>());\n";
			}
			tmpl << "	}\n";
		}
		return tmpl.str();
	}

	string GetStaticMethodImpl(const Class& c)
	{
		map<string, vector<Function>> methods_by_name = GetMethodsByName(
				c.staticMethods);

		stringstream tmpl;
		bool first = true;
		for (const auto& methods : methods_by_name)
		{
			tmpl << "	";
			if (first)
			{
				first = false;
			}
			else
			{
				tmpl << "else ";
			}
			tmpl << "if (name == \"" << methods.first << "\")\n";
			tmpl << "	{\n";
			for (const auto& method : methods.second)
			{
				string name = c.getFullName() + "::" + methods.first;
				tmpl << "		results.push_back(std::make_unique<Function<"
					<< serializer::GetFunctionSignature(method) << ", " << name
					<< ">>());\n";
			}
			tmpl << "	}\n";
		}
		return tmpl.str();
	}

	string StaticMethodsDefinitions(const Class& c)
	{
		if (c.staticMethods.empty())
		{
			return string();
		}

		stringstream tmpl;
		tmpl << "// " << c.getFullName() << " static methods definitions.\n";

		for (const auto& method : c.staticMethods)
		{
			serializer::SerializeFunctionSources(tmpl, method);
		}

		tmpl << "// End of " << c.getFullName()
			<< " static methods definitions.\n";

		return tmpl.str();
	}

	string GetFieldImpl(
			const Class::Fields& fields, const string& field_prefix)
	{
		stringstream tmpl;
		for (const auto& field : fields)
		{
			tmpl << "		if (name == \"" << field.name << "\")\n";
			tmpl << "		{\n";
			tmpl << "			return Reference("
				<< field_prefix << field.name << ");\n";
			tmpl << "		}\n";
		}
		return tmpl.str();
	}
}

void serializer::SerializeClassHeader(ostream& o, const Class& c)
{
	stringstream tmpl;
	tmpl << R"(
template <>
class Class<%name%> : public IClass
{
public:
	static const constexpr int FieldCount = %field_count%;
	static const constexpr int StaticFieldCount = %static_field_count%;
	static const constexpr int MethodCount = %method_count%;
	static const constexpr int StaticMethodCount = %static_method_count%;

	int GetFieldCount() const override;
	Reference GetField(
			const Reference& o, const std::string& name) const override;

	int GetStaticFieldCount() const override;
	Reference GetStaticField(const std::string& name) const override;

	int GetMethodCount() const override;
	std::vector<std::unique_ptr<IMethod>> GetMethod(
			const std::string& name) const override;

	int GetStaticMethodCount() const override;
	std::vector<std::unique_ptr<IFunction>> GetStaticMethod(
			const std::string& name) const override;

	const std::string& getName() const override;

	// Calls T::operator() on each field of '%name%'.
	// Works well with C++14 generic lambdas.
	template <typename T>
	static void IterateFields(const %name%& c, T t);

	template <typename T>
	static void IterateFields(%name%& c, T t);

	template <typename T>
	static void IterateStaticFields(T t);
};

template <typename T>
void Class<%name%>::IterateFields(const %name%& c, T t)
{
%iterate_fields%}

template <typename T>
void Class<%name%>::IterateFields(%name%& c, T t)
{
%iterate_fields%}

template <typename T>
void Class<%name%>::IterateStaticFields(T t)
{
%iterate_static_fields%}

%methods_decl%%static_methods_decl%
)";

	o << replaceAll(
			tmpl.str(),
			{
					{"%name%",                  c.getFullName()},
					{"%iterate_fields%",        IterateFields(c)},
					{"%iterate_static_fields%", IterateStaticFields(c)},
					{"%field_count%",           to_string(c.fields.size())},
					{"%static_field_count%",    to_string(c.staticFields.size())},
					{"%method_count%",          to_string(c.methods.size())},
					{"%methods_decl%",          MethodsDeclarations(c)},
					{"%static_method_count%",   to_string(c.staticMethods.size())},
					{"%static_methods_decl%",   StaticMethodsDeclarations(c)}
			});
}

void serializer::SerializeClassSources(ostream& o, const Class& c)
{
	stringstream tmpl;
	tmpl << R"(
const int Class<%name%>::FieldCount;
const int Class<%name%>::StaticFieldCount;
const int Class<%name%>::MethodCount;
const int Class<%name%>::StaticMethodCount;

int Class<%name%>::GetFieldCount() const
{
	return FieldCount;
}

Reference Class<%name%>::GetField(const Reference& r, const std::string& name) const
{)";
	if (!c.fields.empty())
	{
		tmpl << R"(
	if (r.IsT<%name%>())
	{
		%name%& o = r.GetT<%name%>();
%get_field_impl%	}
	else if (r.IsT<const %name%>())
	{
		const %name%& o = r.GetT<const %name%>();
%get_field_impl%	}
	else
	{
		throw Exception("Invalid Reference passed to GetField().");
	})";
	}
	tmpl << R"(
	throw Exception("Invalid name passed to GetField().");
}

int Class<%name%>::GetStaticFieldCount() const
{
	return StaticFieldCount;
}

Reference Class<%name%>::GetStaticField(const std::string& name) const
{
%get_static_field_impl%	throw Exception("Invalid name passed to GetStaticField().");
}

int Class<%name%>::GetMethodCount() const
{
	return MethodCount;
}

std::vector<std::unique_ptr<IMethod>> Class<%name%>::GetMethod(const std::string& name) const
{
	std::vector<std::unique_ptr<IMethod>> results;
%get_method_impl%
	return results;
}

int Class<%name%>::GetStaticMethodCount() const
{
	return StaticMethodCount;
}

std::vector<std::unique_ptr<IFunction>> Class<%name%>::GetStaticMethod(
		const std::string& name) const
{
	std::vector<std::unique_ptr<IFunction>> results;
%get_static_method_impl%
	return results;
}

static const std::string %escaped_name%_name = "%name%";

const std::string& Class<%name%>::getName() const
{
	return %escaped_name%_name;
}

%method_definitions%%static_method_definitions%

namespace
{
	// Object to auto-register %name%.
	struct %escaped_name%_registrar
	{
		%escaped_name%_registrar()
		{
			::reflang::registry::internal::Register(
					std::make_unique<Class<%name%>>());
		}
	} %escaped_name%_instance;
})";

	o << replaceAll(
			tmpl.str(),
			{
					{"%name%",                      c.getFullName()},
					{"%get_field_impl%",            GetFieldImpl(c.fields, "o.")},
					{
					 "%get_static_field_impl%",
					                                GetFieldImpl(c.staticFields, c.getFullName() + "::")
					},
					{"%field_count%",               to_string(c.fields.size())},
					{"%escaped_name%",              getNameWithoutColons(c.getFullName())},
					{"%method_definitions%",        MethodsDefinitions(c)},
					{"%get_method_impl%",           GetMethodImpl(c)},
					{"%static_method_definitions%", StaticMethodsDefinitions(c)},
					{"%get_static_method_impl%",    GetStaticMethodImpl(c)}
			});
}
