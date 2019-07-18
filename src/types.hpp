#ifndef REFLANG_TYPES_HPP
#define REFLANG_TYPES_HPP

#include <string>
#include <vector>

#include <ostream>
#include <memory>


struct CxxType {
	//	using Kind = CXTypeKind;
	//	Kind kind;
	explicit CxxType(std::string spelling, bool ptr_ = false, bool ref_ = false, bool const_ = false,
			std::string mangling = "", std::string pointee = "")
			: type_name_(move(spelling))
			  , ctype_name_(mangling)
			  , pointee_(pointee)
			  , is_ptr_(ptr_)
			  , is_ref_(ref_)
			  , is_const_(const_)
			{}

	CxxType() = default;
	CxxType(const CxxType&) = default;
	CxxType(CxxType&&) = default;
	~CxxType() = default;
	CxxType& operator=(const CxxType&) = default;
	CxxType& operator=(CxxType&&) = default;

	// OK to return the reference to member, as lifetime of objects stored in type registry is infinite.
	// Or ??? in fact i don't want _any_ temporary strings, even with move semantics
	std::string asCType() const;      // ex: Namespace__Class__InnerClass const * foo
	std::string asCxxType() const { return type_name_; }    // ex: Namespace::Class::InnerClass const & foo
	std::string pointee() const { return pointee_; }

	bool isRef() const { return is_ref_; }
	bool isPtr() const { return is_ptr_; }
	bool isPtrOrRef() const { return is_ptr_ || is_ref_; }
	bool isConst() const { return is_const_; }

	bool isMangled() const { return !ctype_name_.empty() && ctype_name_ != type_name_ ; } // TODO do it better

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
	std::string type_name_; // spelling name
	std::string ctype_name_; // mangling
	std::string pointee_;   // C++ native pointee type
//	std::unique_ptr<std::string> mangling_;
	bool is_ptr_ = false;
	bool is_ref_ = false;
	bool is_const_ = false;
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

protected:
	std::string file_;  // TODO make it link to the global set
	std::string full_name_;
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
	bool isConstMember() const { return const_member_; }

	std::string asCName() const;  //!< C mangling

	std::string name;
	Arguments arguments;
	CxxType returnType;
	std::unique_ptr<CxxType> receiver; // hidden argument


private:
	std::string mangling_;
	bool const_member_ = false;
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
