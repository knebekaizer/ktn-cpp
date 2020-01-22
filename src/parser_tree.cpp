#include "parser_tree.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <clang-c/Index.h>

#include "trace.h"

#pragma clang diagnostic ignored "-Wunused-function"

using namespace std;
//using namespace ktn;

namespace
{

CXTranslationUnit parse(
		CXIndex& index, const string& file, const Args& args)
{
	// stupid conversions char* to string and now back to char*
	const char* tmp[args.size()];
	transform(args.begin(), args.end(), tmp, [](auto const& s) {return s.c_str();});

	CXTranslationUnit unit = clang_parseTranslationUnit(
			index,
			file.c_str(), &tmp[0], args.size(),
			nullptr, 0,
			CXTranslationUnit_None);
	if (unit == nullptr)
	{
		cerr << "Unable to parse translation unit. Quitting." << endl;
		exit(-1);
	}

	auto diagnostics = clang_getNumDiagnostics(unit);
	if (diagnostics != 0)
	{
		cerr << "> Diagnostics:" << endl;
		for (int i = 0; i != diagnostics; ++i)
		{
			auto diag = clang_getDiagnostic(unit, i);
			log_error << ">>> "
				<< clang_formatDiagnostic(
						diag, clang_defaultDiagnosticDisplayOptions());
			exit(-1);
		}
	}

	return unit;
}

}  // namespace

struct XCursor;

string str(CXCursorKind k) { return XString(k); }
string str(CXTypeKind k) { return XString(k); }

struct Entity {
	string          usr;
	string          name;
	CXCursorKind    cursorKind = (CXCursorKind)0;
	CXTypeKind      typeKind = CXType_Invalid;
	string          typeName;

	Entity() = default;
	explicit Entity(const XCursor& c);

	string cursorKindS() const { return str(cursorKind); }
	string typeKindS() const { return str(typeKind); }
};


struct XCursor : CXCursor {
	XCursor(const CXCursor& c) : CXCursor(c) {}

	string spelling() const { return XString(*this); }
	auto kind() const { return clang_getCursorKind(*this); }
	auto type() const { return clang_getCursorType(*this); }
	string typeName() const { return XString(type()); }
	string usr() const { return XString(clang_getCursorUSR(*this)); }

	Entity data() const { return Entity(*this); }
};

struct XType : CXType {
	XType(const CXType& t) : CXType(t) {}
	string name() const { return XString(*this); }
	string kindS() const { return XString(kind); }
};

Entity::Entity(const XCursor& c)
		: usr{ c.usr() }
		, name { c.spelling() }
		, cursorKind {c.kind() }
		, typeKind { c.type().kind }
		, typeName { c.typeName() }
{}


using Strings = vector<string>;

ostream& operator<<(ostream& os, Entity const& c) {
	return os << c.name << " : " << c.typeName << " of " << c.typeKindS();
}

ostream& operator<<(ostream& os, XCursor const& c) {
	return os << c.data();
}

ostream& operator<<(ostream& os, Strings const& out) {
	for (auto const& x : out) os << x << endl;
	return os;
}

class Node;
class Container;
struct Tree;
struct Namespace;
struct Struct;
struct Function;
struct CXXMethod;
struct Constructor;
struct Var;
struct Field;
struct Enum;


struct Render {
public:
	virtual Render& renderNode(const Node&) { return *this; }
	virtual Render& renderContainer(const Container& x) { return *this; }
	virtual Render& renderTree(const Tree& x);
	virtual Render& renderNamespace(const Namespace& x);
	virtual Render& renderStruct(const Struct& x);
	virtual Render& renderFunc(const Function& x);
	virtual Render& renderCXXMethod(const CXXMethod& x);
	virtual Render& renderConstructor(const Constructor& x);
	virtual Render& renderVar(const Var& x);
	virtual Render& renderField(const Field& x);
	virtual Render& renderEnum(const Enum& x);
	vector<string> lines;
};

class Node {
protected:
	explicit Node() = default;
public:
	enum class Kind {Dummy, Namespace, Struct, Function, Variable} ;

	Node(Entity n, const Container& p) : data(n), parent(&p) {}

	Entity    data;
	const Container* parent = nullptr;

	virtual Render& accept(Render& r) const { return r.renderNode(*this); }

	string usr() const { return data.usr; }
	virtual string name() const { return data.name; };
	virtual string kindSpelling() const = 0;

	virtual ~Node() = default;
};


ostream& operator<<(ostream& os, Render const& r) {
	for (auto& x : r.lines) {
		os << x << endl;
	}
	return os;
}

class Container : public Node {
protected:
	explicit Container() = default;

public:
	Container(const Entity& data, const Container& p) : Node(data, p) {}
	using Nodes = vector<unique_ptr<Node>>;

	Nodes children;
	virtual void add(Node* n) { children.emplace_back(n); }

	Node* get(string usr) const {
		for (auto& x : children) {
			if (x->usr() == usr) return x.get();
		}
		return nullptr;
	}

	Render& accept(Render& r) const override { return r.renderContainer(*this); }

	string kindSpelling() const override { return "NotSupported"; };
};


struct Log : Render {
	Log& renderNode(const Node& c) override;
	Log& renderContainer(const Container& c) override;

	struct Indenter{
		Indenter(int& i) : indent(++i) {}
		~Indenter() { --indent; }
		int& indent;
	};
	int indent = 0;
};

Log& Log::renderNode(const Node& c) {
	string prefix = string(indent * 4, ' ');
	lines.push_back( prefix + "." + c.name() + " : " + c.kindSpelling() );
	return *this;
}

Log& Log::renderContainer(const Container& x) {
	renderNode(x); // prolog
	for (auto& c : x.children) {
		Indenter i(indent);
		c->accept(*this);
	}
	// no epilog
	return *this;
}

ostream& operator<<(ostream& os, Node const& x) {
	Log log;
	x.accept(log);
	return os << log << endl;
}


struct Function : Node {
	Function(const Entity& data, const Container& p) : Node(data, p) {}

	string kindSpelling() const override { return "Function"; };
	Render& accept(Render& r) const override { return r.renderFunc(*this); }
};

struct CXXMethod : Function {
	CXXMethod(const Entity& data, const Container& p) : Function(data, p) {}

	string kindSpelling() const override { return "Function"; };
	Render& accept(Render& r) const override { return r.renderCXXMethod(*this); }
};

struct Constructor : Function {
	Constructor(const Entity& data, const Container& p) : Function(data, p) {}
	string kindSpelling() const override { return "Constructor"; };
	Render& accept(Render& r) const override { return r.renderConstructor(*this); }
};

struct Var : Node {
	Var(const Entity& data, const Container& p) : Node(data, p) {}
	string kindSpelling() const override { return "Variable"; };
	Render& accept(Render& r) const override { return r.renderVar(*this); }
};

struct Field : Node {
	Field(const Entity& data, const Container& p) : Node(data, p) {}
	string kindSpelling() const override { return "Field"; };
	Render& accept(Render& r) const override { return r.renderField(*this); }
};

struct Enum : Node {
	Enum(const Entity& data, const Container& p) : Node(data, p) {}
	string kindSpelling() const override { return "Eum"; };
	Render& accept(Render& r) const override { return r.renderEnum(*this); }
};

struct Namespace : public Container {
public:
	Namespace(const Entity& data, const Container& p) : Container(data, p) {
		log_trace << "Namespace ctor: " << data.name;
	}
	string kindSpelling() const override { return "Namespace"; };
	Render& accept(Render& r) const override { return r.renderNamespace(*this); }
};

struct Struct : public Container {
public:
	Struct(const Entity& data, const Container& p) : Container(data, p) {}
	string kindSpelling() const override { return "Struct"; };
	string name() const override { return data.typeName; };
	Render& accept(Render& r) const override { return r.renderStruct(*this); }
};

struct Tree : public Container {
public:
	string kindSpelling() const override { return "Root"; };
	Render& accept(Render& r) const override { return r.renderTree(*this); }
};

Render& Render::renderTree(const Tree& x) { return renderContainer(x); }
Render& Render::renderNamespace(const Namespace& x) { return renderContainer(x); }
Render& Render::renderStruct(const Struct& x) { return renderContainer(x); }
Render& Render::renderFunc(const Function& x) { return renderNode(x); }
Render& Render::renderCXXMethod(const CXXMethod& x) { return renderNode(x); }
Render& Render::renderConstructor(const Constructor& x) { return renderNode(x); }
Render& Render::renderVar(const Var& x) { return renderNode(x); }
Render& Render::renderField(const Field& x) { return renderNode(x); }
Render& Render::renderEnum(const Enum& x) { return renderNode(x); }


CXChildVisitResult typesVisitor(CXCursor c, CXCursor _, CXClientData client_data)
{
	assert(client_data);
	const XCursor& cursor(c);
	Container& parent(*reinterpret_cast<Container*>(client_data));

	XCursor semanticParent = clang_getCursorSemanticParent(cursor);
	if (semanticParent.usr() != parent.usr()) {
		log_trace << "P: this{" << cursor << "}; parent{" << semanticParent << "}; container{" << parent << "}";
	//	Trace2(semanticParent, parent); // Trace2(semanticParent.spelling(), parent.name());
	}

	auto entity = cursor.data();
	switch (cursor.kind()) {
		case CXCursor_Namespace: {
				if (auto x = parent.get(entity.usr)) {
					clang_visitChildren(cursor, typesVisitor, x);
				} else if (auto x = new Namespace(cursor.data(), parent)) {
					clang_visitChildren(cursor, typesVisitor, x);
					parent.add(x);
				}
			}
			break;

		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
		case CXCursor_UnionDecl:
			if (auto x = new Struct(cursor.data(), parent)) {
				clang_visitChildren(cursor, typesVisitor, x);
				parent.add(x);
				TraceX(cursor, *x);
			}
			break;

		case CXCursor_ClassTemplate:
			if (auto x = new Container(cursor.data(), parent)) {
				clang_visitChildren(cursor, typesVisitor, x);
				parent.add(x);
			}
			break;

		case CXCursor_Constructor:
			if (auto x = new Constructor(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_CXXMethod:
			if (auto x = new Function(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_FunctionDecl:
			if (auto x = new Function(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_FunctionTemplate:
			break;

		case CXCursor_VarDecl:
			if (auto x = new Var(cursor.data(), parent)) {
				XType canonT = clang_getCanonicalType(cursor.type());
				auto sizeOfT = clang_Type_getSizeOf(cursor.type());
				if (canonT.kind == CXType_Vector) {
					XType elemT = clang_getElementType(canonT);
					TraceX(clang_getArraySize(canonT));
					Trace2(x->name(), sizeOfT);
					Trace2(elemT.name(), elemT.kindS());
					Trace3(x->name(), x->data.cursorKindS(), x->data.typeKindS());
					Trace3(x->name(), canonT.name(), canonT.kindS());
				}
				auto align = clang_Type_getAlignOf(cursor.type());
				TraceX(x->name(), sizeOfT, align, cursor.typeName());

				parent.add(x);
			}
			break;

		case CXCursor_FieldDecl:
			if (auto x = new Field(cursor.data(), parent)) {
                auto offset = clang_Type_getOffsetOf(semanticParent.type(), x->name().c_str());
                auto align = clang_Type_getAlignOf(cursor.type());
                TraceX(x->name(), offset, align);
				parent.add(x);
			}
			break;

		case CXCursor_EnumDecl:
			if (auto x = new Enum(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		default:
			return CXChildVisit_Recurse;
	}
	return CXChildVisit_Continue;
}



void  parseTypes(
		const std::vector<std::string>& files,
        const Args& args //, const Options&  options
		)
{
	for (const auto& file : files)
	{
		CXIndex index =  clang_createIndex(0, 0);
		CXTranslationUnit unit = parse(index, file, args);

		auto cursor = clang_getTranslationUnitCursor(unit);

		Tree tree;
		clang_visitChildren(cursor, typesVisitor, &tree);

		cout << tree;

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}
}
