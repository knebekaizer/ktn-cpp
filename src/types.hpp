#ifndef REFLANG_TYPES_HPP
#define REFLANG_TYPES_HPP

#include <string>
#include <vector>

#include <ostream>
#include <memory>


struct CxxType {
	//	using Kind = CXTypeKind;
	//	Kind kind;
	explicit CxxType(std::string spelling, bool ref_ = false, bool const_ = false)
			: type(move(spelling)), isRef_(ref_), isConst_(const_) {}

	CxxType() = default;
	CxxType(const CxxType&) = default;
	CxxType(CxxType&&) = default;
	~CxxType() = default;
	CxxType& operator=(const CxxType&) = default;
	CxxType& operator=(CxxType&&) = default;

	std::string asCType() const;      // ex: Namespace__Class__InnerClass const * foo
	std::string asCxxType() const { return type; }    // ex: Namespace::Class::InnerClass const & foo

	bool isRef() const { return isRef_; }

	bool isConst() const { return isConst_; }

	std::string type; // spelling name
	/*
	shortName    // name in local scope: foo
	fullName     // qualified: Namespace::Class::foo
	realName     // canonical
	cName        // mangling, uniq
	internalName // mangling, uniq
		Attr:
	 isConst
	 isRef
	 isPtr
	 isCType  // primitive or pointer to CType
	 */

private:
	bool isRef_;
	bool isConst_;
};

class TypeBase {
public:
	enum class Type {
		Enum,
		Function,
		Class,
	};

public:
	TypeBase(std::string file, std::string full_name);

	virtual ~TypeBase();

	virtual Type getType() const = 0;
	const std::string& fullName() const;
	const std::string& getName() const;
	const std::string& getFile() const;
	const std::string& asCName() const;  //!< C mangling

protected:
	std::string file_;  // TODO make it link to the global set
	std::string full_name_;
	std::string mangling_;
};

class Enum : public TypeBase {
public:
	using Values = std::vector<std::pair<std::string, int>>;

public:
	Enum(std::string file, std::string full_name);

	Type getType() const override;

	Values values;
};

struct NamedObject {
	std::string name;
	std::string type;
};

struct TypedName : CxxType {
	TypedName(const std::string& n, CxxType&& type)
			: CxxType(type), name(n) {}

	/*const*/ std::string name;
};

inline std::ostream& operator<<(std::ostream& os, CxxType const& x) {
	return os << x.asCxxType(); // default printing
}

class Class;

class Function : public TypeBase {
public:
	using Argument = TypedName;
	using Arguments = std::vector<Argument>;

	Function(std::string file, std::string full_name, bool constMember = false);

	Type getType() const override;

	void setReceiver(CxxType&& thiz); // set optional receiver, i.e. class type if the function is non-static class member
	bool isInstanceMember() const { return receiver ? true : false; }

	std::string name;
	Arguments arguments;
	CxxType returnType;
	std::unique_ptr<CxxType> receiver; // hidden argument
};

// Non-static member function
class Method : public Function {
	CxxType receiver;
};

// Factory create matching a constructor
class Ctor : public Function {
};

// Deleter matching the destructor
class Dtor : public Function {
};


class Class : public TypeBase {
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

	Fields fields;
	Fields staticFields;
};

namespace std {
std::ostream& operator<<(std::ostream& os, const NamedObject& x);

std::ostream& operator<<(std::ostream& os, const Function& f);

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& list) {
	for (auto k = 0; k != list.size(); ++k) {
		os << list[k];
		if (k < list.size() - 1) os << ", "; /// FIXME
	}
	return os;
}
}

template<typename T>
std::ostream& prettyPrint(std::ostream& os, const std::vector<T>& list, const std::string& sep = ", ") {
	for (auto k = 0; k != list.size(); ++k) {
		os << list[k];
		if (k < list.size() - 1) os << sep;
	}
	return os;
}

inline std::ostream& std::operator<<(std::ostream& os, const Function& f) {
	return os << f.returnType << " " << f.fullName() << "(" << f.arguments << ")";
}

inline std::ostream& std::operator<<(std::ostream& os, const NamedObject& x) {
	return os << x.type << " " << x.name;
}

inline std::ostream& operator<<(std::ostream& os, const Class& c) {
	os << c.fullName() << "{\n";
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
	os << "}\n";
	return os;
}

#endif //REFLANG_TYPES_HPP
