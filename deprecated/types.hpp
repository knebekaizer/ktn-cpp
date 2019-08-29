#ifndef REFLANG_TYPES_HPP
#define REFLANG_TYPES_HPP

#include <string>
#include <vector>

#include <ostream>
#include <sstream>
#include <memory>
#include <cstddef>

class CxxType {
public:
	enum class KIND { // keep a minimal subset of CxTypeKind (remapped)
		OTHER, INVALID, VOID, PTR, REF, POD
	};
	//	using Kind = CXTypeKind;
	//	Kind kind;
	explicit CxxType(std::string spelling, std::size_t size, KIND kind = KIND::OTHER, bool const_ = false,
			std::string mangling = "", std::string pointee = "")
			: type_name_(move(spelling))
			  , ctype_name_(std::move(mangling))
			  , pointee_(std::move(pointee))
			  , kind_(kind)
			  , is_const_(const_)
			  , size_(size)
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

	auto size() const { return size_; }

	KIND kind() const { return kind_; }
	bool isConst() const { return is_const_; }
	bool isRef() const { return kind_ == KIND::REF; }
	bool isPtr() const { return kind_ == KIND::PTR; }
	bool isVoid() const { return kind_ == KIND::VOID; }

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

	// Debug
	std::string dump() const {
		std::ostringstream os;
		os << "CxxType{" << asCxxType()
		   << " ("
		   << (isConst() ? "const" : "")
		   << (isPtr() ? "*" : "")
		   << (isRef() ? "&" : "")
		   <<") "
		   << "C{" << asCType() << "} "
		   << (isPtr() || isRef() ? (std::string(" -> ") + pointee()) : std::string())
		   << "}";
		return os.str();
	}

private:
	std::string type_name_; // spelling name
	std::string ctype_name_; // mangling
	std::string pointee_;   // C++ native pointee type
	KIND kind_;
	bool is_const_ = false;
	std::size_t size_ = 0;
};


inline std::ostream& operator<<(std::ostream& os, CxxType const& x) {
	return os << x.asCxxType(); // default printing
}

inline std::ostream& operator<<(std::ostream& os, CxxType::KIND const& x) {
	static const char* names[] = {"OTHER", "INVALID", "VOID", "PTR", "REF", "POD"};
	// weak attempt to keep the map consistent
	static_assert((int)CxxType::KIND::REF == 4, "FIXME: CxxType::KIND layout have been changed recently");
//	OTHER, INVALID, VOID, PTR, REF, POD
	os << (((unsigned int)x < sizeof(names)/sizeof(*names)) ? names[(unsigned int)x] : "UNKNOWN");
	return os; // << x.asCxxType(); // default printing
}


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

	std::string asCName() const { return !mangling_.empty() ? mangling_ : fullName(); }
	std::string shortName() const { return short_name; }

	Arguments arguments;
	CxxType returnType;
	std::unique_ptr<CxxType> receiver; // hidden argument

	std::string short_name;
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