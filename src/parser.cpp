#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.class.hpp"
#include "parser.enum.hpp"
#include "parser.function.hpp"
#include "parser.util.hpp"

#include "generator.h"

#include "trace.h"

#pragma clang diagnostic ignored "-Wunused-function"

using namespace std;
using namespace ktn;

namespace
{

CXTranslationUnit parse(
		CXIndex& index, const string& file, int argc, char** argv)
{
	CXTranslationUnit unit = clang_parseTranslationUnit(
			index,
			file.c_str(), argv, argc,
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
		//	exit(-1);
		}
	}

	return unit;
}

struct GetTypesStruct
{
	vector<unique_ptr<TypeBase>>* types;
	const ktn::Options* options;
};



CXChildVisitResult getTypesVisitor(
		CXCursor cursor, CXCursor parent, CXClientData client_data)
{

	auto* data = reinterpret_cast<GetTypesStruct*>(client_data);

	// Awfull. FIXME registry
	static unordered_set<string> types_registry;   // classes & structs
	static unordered_set<string> symbols_registry; // var & functions
	static WildCard path(data->options->path_filter);

	if (!path.match(ktn::getFile(cursor))) return CXChildVisit_Recurse;  // filter out "external" types

log_info << clang_getCursorKindSpelling(clang_getCursorKind(cursor));

	std::unique_ptr<TypeBase> type;
	switch (clang_getCursorKind(cursor)) {
		case CXCursor_EnumDecl:
			type = std::make_unique<Enum>(ktn::buildEnum(cursor));
			break;
		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
			if (!types_registry.insert(buildFullName(cursor)).second) break;  // skip dups
			type = std::make_unique<Class>(ktn::buildClass(cursor));
			break;
		case CXCursor_FunctionDecl:
			if (isOperatorFunction(cursor)) break;
			if (!symbols_registry.insert( convertAndDispose(clang_Cursor_getMangling(cursor)) ).second) break;  // skip dups
			type = std::make_unique<Function>(ktn::buildFunction(cursor));
			break;
		case CXCursor_VarDecl:
		//	if (!types_registry.insert( convertAndDispose(clang_Cursor_getMangling(cursor)) ).second) break;  // skip dups
{
	auto t = clang_getCursorType (cursor);
log_trace << "Var: " <<  clang_getCursorSpelling(cursor) << "> "
     << "\n type: " <<  t
	 << "\n canonical: " << clang_getCanonicalType(t)
	 << "\n pointee: " << clang_getPointeeType(t)
     << "\n elemKind: " << clang_getElementType(t) ;
}
		default:
			break;
	}

	if (type) {
		const string& name = type->fullName();
		if (type
		    && !name.empty()
		    && ktn::isRecursivelyPublic(cursor)
		    && name.back() != ':'
		    && path.match(type->getFile())
		    && regex_match(name, data->options->include)
		    && !regex_match(name, data->options->exclude))
		{
			data->types->push_back(std::move(type));
		}
	}

	return CXChildVisit_Recurse;
}

}  // namespace

struct Cursor;

struct Entity {
	string          usr;
	string          name;
	CXCursorKind    kind = (CXCursorKind)0;
	CXTypeKind      typeKind = CXType_Invalid;
	string          typeName;

	Entity() = default;
	Entity(const Cursor& c);
	
	string sTypeKind() const { return convertAndDispose(clang_getTypeKindSpelling(typeKind)); }
	string sKind() const { return convertAndDispose(clang_getCursorKindSpelling(kind)); }
};


struct Cursor : CXCursor {
	Cursor(const CXCursor& c) : CXCursor(c) {}
	
	string spelling() const { return convertAndDispose(clang_getCursorSpelling(*this)); }
	auto kind() const { return clang_getCursorKind(*this); }
	auto type() const { return clang_getCursorType(*this); }
	auto typeName() const { return convertAndDispose(clang_getTypeSpelling(type())); }
	auto usr() const { return convertAndDispose(clang_getCursorUSR(*this)); }

	Entity data() const { return Entity(*this); }
};

Entity::Entity(const Cursor& c)
		: usr{ c.usr() }
		, name { c.spelling() }
		, kind { c.kind() }
		, typeKind { c.type().kind }
		, typeName { c.typeName() }
{}



using Strings = vector<string>;

ostream& operator<<(ostream& os, Entity const& c) {
	return os << c.name << " : " << c.typeName << " of " << c.sTypeKind();
}

ostream& operator<<(ostream& os, Cursor const& c) {
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
struct Func;
struct CXXMethod;
struct Constructor;
struct Var;
struct Field;
struct Enum_;


struct Render {
public:
	virtual Render& renderNode(const Node&) { return *this; }
	virtual Render& renderContainer(const Container& x) { return *this; }
	virtual Render& renderTree(const Tree& x);
	virtual Render& renderNamespace(const Namespace& x);
	virtual Render& renderStruct(const Struct& x);
	virtual Render& renderFunc(const Func& x);
	virtual Render& renderCXXMethod(const CXXMethod& x);
	virtual Render& renderConstructor(const Constructor& x);
	virtual Render& renderVar(const Var& x);
	virtual Render& renderField(const Field& x);
	virtual Render& renderEnum_(const Enum_& x);
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
	using Nodes = vector<unique_ptr<const Node>>;

	Nodes children;
	virtual void add(const Node* n) { children.emplace_back(n); }

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


struct Func : Node {
	Func(const Entity& data, const Container& p) : Node(data, p) {}

	string kindSpelling() const override { return "Function"; };
	Render& accept(Render& r) const override { return r.renderFunc(*this); }
};

struct CXXMethod : Func {
	CXXMethod(const Entity& data, const Container& p) : Func(data, p) {}

	string kindSpelling() const override { return "Function"; };
	Render& accept(Render& r) const override { return r.renderCXXMethod(*this); }
};

struct Constructor : Func {
	Constructor(const Entity& data, const Container& p) : Func(data, p) {}
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

struct Enum_ : Node {
	Enum_(const Entity& data, const Container& p) : Node(data, p) {}
	string kindSpelling() const override { return "Eum"; };
	Render& accept(Render& r) const override { return r.renderEnum_(*this); }
};

struct Namespace : public Container {
public:
	Namespace(const Entity& data, const Container& p) : Container(data, p) {}
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
Render& Render::renderFunc(const Func& x) { return renderNode(x); }
Render& Render::renderCXXMethod(const CXXMethod& x) { return renderNode(x); }
Render& Render::renderConstructor(const Constructor& x) { return renderNode(x); }
Render& Render::renderVar(const Var& x) { return renderNode(x); }
Render& Render::renderField(const Field& x) { return renderNode(x); }
Render& Render::renderEnum_(const Enum_& x) { return renderNode(x); }


CXChildVisitResult typesVisitor(CXCursor c, CXCursor _, CXClientData client_data)
{
	assert(client_data);
	const Cursor& cursor(c);
	Container& parent(*reinterpret_cast<Container*>(client_data));

	Cursor p = clang_getCursorSemanticParent(cursor);
	if (p.usr() != parent.usr()) {
		log_trace << "P: this{" << cursor << "}; parent{" << p << "}; container{" << parent << "}";
	//	Trace2(p, parent); // Trace2(p.spelling(), parent.name());
	}

	switch (cursor.kind()) {
		case CXCursor_Namespace:
			if (auto x = new Namespace(cursor.data(), parent)) {
				clang_visitChildren(cursor, typesVisitor, x);
				parent.add(x);
			}
			break;

		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
		case CXCursor_UnionDecl:
			if (auto x = new Struct(cursor.data(), parent)) {
				clang_visitChildren(cursor, typesVisitor, x);
				parent.add(x);
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
			if (auto x = new Func(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_FunctionDecl:
			if (auto x = new Func(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_FunctionTemplate:
			break;

		case CXCursor_VarDecl:
			if (auto x = new Var(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_FieldDecl:
			if (auto x = new Field(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_EnumDecl:
			if (auto x = new Enum_(cursor.data(), parent)) {
				parent.add(x);
			}
			break;

		default:
			return CXChildVisit_Recurse;
	}
	return CXChildVisit_Continue;
}


vector<string> ktn::getSupportedTypeNames(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options)
{
	auto types = getTypes(files, argc, argv, options);

	vector<string> names;
	names.reserve(types.size());
	for (const auto& type : types)
	{
		names.push_back(type->fullName());
	}
	return names;
}




vector<unique_ptr<TypeBase>> ktn::  getTypes(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options,
		WrapperGenerator* gen)
{
//	for (int k=0; k!= argc; ++k) log_trace << argv[k];
	vector<unique_ptr<TypeBase>> results;
	auto last = 0;
	for (const auto& file : files)
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = parse(index, file, argc, argv);

		auto cursor = clang_getTranslationUnitCursor(unit);

//		GetTypesStruct data = { &results, &options };
//		clang_visitChildren(cursor, getTypesVisitor, &data);
//
		Tree tree;
		clang_visitChildren(cursor, typesVisitor, &tree);

		cout << tree;


		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	//	log_debug << "Loaded " << results.size() << " types";
//		if (gen) gen->genWrappers(results.begin() + last, results.end());
		last = results.size();
	}
	return results;
}
