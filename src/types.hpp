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
		const std::string& asCName() const;  //!< C mangling

	protected:
		std::string file_;
		std::string full_name_;
		std::string mangling_;
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

	struct CxxType
	{
	//	using Kind = CXTypeKind;
	//	Kind kind;
		CxxType(const std::string& spelling, bool isRef = false, bool isConst = false)
			: type(spelling)  {}
		CxxType() = default;
		CxxType(const CxxType&) = default;
		CxxType(CxxType&&) = default;
		~CxxType() = default;
		CxxType& operator=(const CxxType&) = default;

		bool isRefType() const { return false; } // not implemented
		bool isConst() const { return false; } // not implemented
		std::string asCType() const;      // ex: Namespace__Class__InnerClass const * foo
		std::string asCxxType() const { return type; }    // ex: Namespace::Class::InnerClass const & foo

		std::string type; // spelling
		std::string canonicalType;
		bool isPointerType;
	};

	struct TypedName : CxxType
	{
		TypedName(CxxType&& type, const std::string& n)
			: CxxType(type), name(n) {}
		/*const*/ std::string name;
	};

	inline std::ostream& operator<<(std::ostream& os, CxxType const& x) {
		return os << x.asCxxType(); // default printing
	}

	class Class;

	class Function : public TypeBase
	{
	public:
		using Argument = TypedName;
	    using Arguments = std::vector<Argument>;
		Function(std::string file, std::string full_name, const Class* receiver = nullptr, bool constMember = false);
		Type getType() const override;

		std::string name;
        Arguments arguments;
		CxxType returnType;
		const Class* memberOf;           //!< Class pointer for member functions or nullptr for non-members
		bool isConst;              //!< true for const member function, ignored for static members and free (non-member) functions
	};

	// Non-static member function
	class Method : public Function
	{
		CxxType receiver;
	};

	// Factory create matching a constructor
	class Ctor : public Function
	{
	};

	// Deleter matching the destructor
	class Dtor : public Function
	{
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

namespace std {
std::ostream& operator<<(std::ostream& os, const reflang::NamedObject& x);
std::ostream& operator<<(std::ostream& os, const reflang::Function& f);

template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& list) {
	for (auto k = 0; k != list.size(); ++k) {
		os << list[k];
		if (k < list.size() - 1) os << ", "; /// FIXME
	}
	return os;
}
}

template <typename T> std::ostream& prettyPrint(std::ostream& os, const std::vector<T>& list, const std::string& sep = ", ") {
    for (auto k = 0; k != list.size(); ++k) {
        os << list[k];
        if (k < list.size() - 1) os << sep;
    }
    return os;
}

inline std::ostream& std::operator<<(std::ostream& os, const reflang::Function& f) {
	return os << f.returnType << " " << f.getFullName() << "(" << f.arguments << ")";
}

inline std::ostream& std::operator<<(std::ostream& os, const reflang::NamedObject& x) {
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
