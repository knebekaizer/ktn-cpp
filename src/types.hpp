#ifndef REFLANG_TYPES_HPP
#define REFLANG_TYPES_HPP

#include <string>
#include <vector>

#include <ostream>

namespace reflang
{
	class TypeBase
	{
	public:
		enum class Type
		{
			Enum,
			Function,
			Class,
		};

	public:
		TypeBase(std::string file, std::string full_name);
		virtual ~TypeBase();

		virtual Type getType() const = 0;
		const std::string& getFullName() const;
		const std::string& getName() const;

		const std::string& getFile() const;
		virtual std::string getCName() const;  //!< C mangling

	private:
		std::string file_;
		std::string full_name_;
	};

	class Enum : public TypeBase
	{
	public:
		using Values = std::vector<std::pair<std::string, int>>;

	public:
		Enum(std::string file, std::string full_name);
		Type getType() const override;

		Values values;
	};

	struct NamedObject
	{
		std::string name;
		std::string type;
	};

	class Function : public TypeBase
	{
	public:
	    using Arguments = std::vector<NamedObject>;
		Function(std::string file, std::string full_name, std::string mangling = "");
		Type getType() const override;
		std::string getCName() const override { return mangling; }

		std::string name;
		const std::string mangling;
        Arguments arguments;
		std::string returnType;
	};

	class Class : public TypeBase
	{
	public:
		using Ctors = std::vector<Function>;
		using Methods = std::vector<Function>;
		using Fields = std::vector<NamedObject>;

	public:
		Class(std::string file, std::string full_name);
		Type getType() const override;

		Ctors ctors;
	//	Function dtor;

		Methods methods;
		Methods staticMethods;

		Fields fields;
		Fields staticFields;
	};
}

std::ostream& operator<<(std::ostream& os, const reflang::NamedObject& x);
std::ostream& operator<<(std::ostream& os, const reflang::Function& f);

template <typename T> std::ostream& prettyPrint(std::ostream& os, const std::vector<T>& list, const std::string& sep = ", ") {
    for (auto k = 0; k != list.size(); ++k) {
        os << list[k];
        if (k < list.size() - 1) os << sep;
    }
    return os;
}

template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& list) {
    for (auto k = 0; k != list.size(); ++k) {
        os << list[k];
        if (k < list.size() - 1) os << ", "; /// FIXME
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const reflang::Function& f) {
    return os << f.returnType << " " << f.getFullName() << "(" << f.arguments << ")";
}

inline std::ostream& operator<<(std::ostream& os, const reflang::NamedObject& x) {
    return os << x.type << " " << x.name;
}

inline std::ostream& operator<<(std::ostream& os, const reflang::Class& c) {
    os << c.getFullName() << "{\n";
    if (!c.methods.empty()) {
        prettyPrint(os, c.methods, ";\n");
        os << ";\n";
    }
    if (!c.fields.empty()) {
        prettyPrint(os, c.fields, ";\n");
        os << ";\n";
    }
    if (!c.staticFields.empty()) {
        os << "// @static \n";
        prettyPrint(os, c.staticFields, ";\n");
        os << ";\n";
    }
    if (!c.staticMethods.empty()) {
        os << "// @static \n";
        prettyPrint(os, c.staticMethods, ";\n");
        os << ";\n";
    }
     os << "}\n";
    return os;
}

#endif //REFLANG_TYPES_HPP
