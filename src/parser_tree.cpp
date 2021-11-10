#include "parser_tree.h"
#include "utils.h"
#include "xclang.hpp"
#include "ast_views.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>

#include "range/v3/all.hpp"
using ranges::to_vector;
namespace rng = ranges;
namespace vs = ranges::views;

#include <clang-c/Index.h>

#include "trace.h"

#pragma clang diagnostic ignored "-Wunused-function"

using namespace std;
//using namespace ktn;

namespace {

CXTranslationUnit parse(
		CXIndex& index, const string& file, const Args& args)
{
//	// stupid conversions char* to string and now back to char*
//	const char* tmp[args.size()];
//	std::transform(args.begin(), args.end(), tmp, [](auto const& s) {return s.c_str();});
	auto argv = args | vs::transform(&string::c_str) | to_vector;

	CXTranslationUnit unit = clang_parseTranslationUnit(
			index,
			file.c_str(), &argv[0], (int)argv.size(),
			nullptr, 0,
			CXTranslationUnit_None);
	if (unit == nullptr) {
		cerr << "Unable to parse translation unit. Quitting." << endl;
		exit(-1);
	}

	auto diagnostics = clang_getNumDiagnostics(unit);
	if (diagnostics != 0) {
		cerr << "> Diagnostics:" << endl;
		int errCount = 0;
		for (int i = 0; i != diagnostics; ++i) {
			auto diag = clang_getDiagnostic(unit, i);
			auto severity = clang_getDiagnosticSeverity(diag);
			if ((int)severity > (int)CXDiagnostic_Warning) errCount++;
			cerr << clang_formatDiagnostic(
						diag, clang_defaultDiagnosticDisplayOptions()) << endl;
		}
		if (errCount > 0) exit(-1);
	}

	return unit;
}

}  // namespace

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
// I don't care what type cursorKind is (scalar or struct ow whatever), so suppress warning for generity
Entity serialize(const XCursor& c) {
	return {
			.usr {c.usr()},
			.name {c.spelling()},
			.cursorKind {c.kind()},
			.typeKind {c.type().kind},
			.typeName {c.typeName()}
	};
}
#pragma clang diagnostic pop


ostream& operator<<(ostream& os, Entity const& c) {
	return os << "["<<c.cursorKindS()<<"] " << c.name << " : " << c.typeName << " of " << c.typeKindS();
}

ostream& operator<<(ostream& os, XCursor const& c) {
	return os << c.data();
}

//ostream& operator<<(ostream& os, Strings const& out) {
//	for (auto const& x : out) os << x << endl;
//	return os;
//}

class Node;
class Container;
struct Tree;
struct Namespace;
struct Struct;
struct ClassTemplate;
struct Function;
struct FunctionTemplate;
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
	virtual Render& renderClassTemplate(const ClassTemplate& x);
	virtual Render& renderFunction(const Function& x);
	virtual Render& renderFunctionTemplate(const FunctionTemplate& x);
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

	Node(const Entity& n, const Container* p) : data(n), parent(p) {}

	Entity	  data;
	const Container* parent = nullptr;

	virtual Render& accept(Render& r) const { return r.renderNode(*this); }

	string usr() const { return data.usr; }
	virtual string name() const { return data.name; };
	virtual string signature() const { return data.typeName + " " + name(); };

//	virtual string kindSpelling() const = 0;
	virtual string kindSpelling() const { return data.typeKindS(); };

	virtual ~Node() = default;
};


ostream& operator<<(ostream& os, Render const& r) {
	// No `endl` after last line
	if (!r.lines.empty()) {
		os << r.lines[0];
		for_each(r.lines.cbegin() + 1, r.lines.cend(), [&os](auto& x){
			os << endl << x;
		});
	}
	return os;
}

class Container : public Node {
protected:
	explicit Container() = default;

public:
	Container(const Entity& data, const Container* p) : Node(data, p) {}
	Container(const XCursor& cursor);

	// FIXME: memory leak. I want unique_ptr for all Node types but shared_ptr for cached ones (see Struct::registy)
//	using Nodes = vector<unique_ptr<Node>>;
	using Nodes = vector<Node*>;

	Nodes children;
	virtual void add(Node* node) { children.emplace_back(node); }

	Node* get(const string& usr) const {
		for (auto& x : children) {
			if (x->usr() == usr) return x; // x.get();
		}
		return nullptr;
	}

	Render& accept(Render& r) const override { return r.renderContainer(*this); }

	string kindSpelling() const override { return data.typeKindS(); };
};

Container::Container(const XCursor& cursor)
	: Container(cursor.data(), nullptr)
{
}


struct Log : Render {
	Log& renderNode(const Node& node) override;
	Log& renderContainer(const Container& container) override;

	struct Indenter{
		explicit Indenter(int& i) : indent(++i) {}
		~Indenter() { --indent; }
		int& indent;
	};
	int indent = 0;
};

Log& Log::renderNode(const Node& node) {
	string prefix = string(indent * 4, ' ');
//	lines.push_back( prefix + "." + node.name() + " : " + node.kindSpelling() );
	ostringstream buf;
	buf << prefix << '[' << node.kindSpelling() << "]\t" << node.signature();
	lines.push_back( buf.str() );
//	lines.push_back( prefix + "." + node.name() + " : " + node.data.typeName << " of " << node.data. kindSpelling() )
//	return os << c.name << " : " << c.typeName << " of " << c.typeKindS();

	return *this;
}

Log& Log::renderContainer(const Container& container) {
	renderNode(container); // prolog
	for (auto& c : container.children) {
		Indenter i(indent);
		c->accept(*this);
	}
	// no epilog
	return *this;
}

struct Json : Render {
	Json& renderNode(const Node& node) override;
	Json& renderContainer(const Container& container) override;
	Json& renderTree(const Tree& x) override;
	Json& renderFunction(const Function& x) override;
private:
	struct Indenter{
		explicit Indenter(int& i) : indent(++i) {}
		~Indenter() { --indent; }
		int& indent;
	};
	int indent = 0;
	void renderNodeOpen(const Node& node);
	void close();
};

void Json::renderNodeOpen(const Node& node) {
	string prefix = string(indent * 4, ' ');
//	lines.push_back( prefix + "." + node.name() + " : " + node.kindSpelling() );
	ostringstream buf_;
#define buf buf_ << prefix
	buf << "{\n";
	prefix += "  ";
	if (auto kind = node.data.cursorKindS(); !kind.empty()) buf << quoted("kind"s) << ": " << quoted(kind) << ",\n";
//	buf << quoted("kind"s) << ": " << node.data.cursorKind << ",\n";
	buf << quoted("name"s) << ": " << quoted(node.data.name) << ",\n";
	if (auto type = node.data.typeName; !type.empty()) buf << quoted("type"s) << ": " << quoted(type) << ",\n";
	buf << quoted("usr"s) << ": " << quoted(node.data.usr) << ",\n";
	buf << quoted("signature"s) << ": " << quoted(node.signature());
//	std::string			usr;
//	CXCursorKind	cursorKind = (CXCursorKind)0;
//	CXTypeKind		typeKind = CXType_Invalid;
//	std::string			typeName;

//	buf << "}"; // keep open

	lines.emplace_back( buf_.str() );
//	lines.push_back( prefix + "." + node.name() + " : " + node.data.typeName << " of " << node.data. kindSpelling() )
//	return os << c.name << " : " << c.typeName << " of " << c.typeKindS();
#undef buf
}

void Json::close() {
	string prefix = string(indent * 4, ' ');
	lines.push_back( prefix + "}," );
}

ostream& operator<<(ostream& os, Node const& x) {
	Log log;
	x.accept(log);
//	return os << log << endl;
	return os << log;
}
//struct Function : Node {
//	Function(const Entity& data, const Container& p) : Node(data, p) {}
struct Function : Container {
	struct Arg {
		using Type = std::string;
		Type type;
		string name;
		Arg(const XType& type_, string name_) : type(type_.name()), name(std::move(name_)) {}
	};
	using Args = vector<Arg>;
	explicit Function(const XCursor& cursor, const Container* p = nullptr);
//	Function(const Entity& data, const Container& p) : Container(data, p) {}
	virtual string signature() const override;

	string kindSpelling() const override { return "Function"; };
	Render& accept(Render& r) const override { return r.renderFunction(*this); }
	Arg::Type retType;
	Args args;
};

string Function::signature() const {
	ostringstream buf;
	buf << retType << ' ';
	buf << name() << '(';
	for (int ix = 0; auto& arg : args) {
		if (ix++) buf << ", ";
		buf << arg.type << ' ' << arg.name;
	}
	buf << ')';
	return buf.str();
}

namespace {

auto genResultType = [](const XCursor& cursor) {
	return ((XType const&)clang_getCursorResultType(cursor)).name();
};

template <class F>
class visitor_fn {
	F fun;
	static CXChildVisitResult go(CXCursor c, CXCursor p, CXClientData client_data) {
		const auto& self = *reinterpret_cast<visitor_fn*>(client_data);
		return self.fun(static_cast<const XCursor&>(c));
	}
public:
	visitor_fn(CXCursor c, F fn) : fun(fn) {
		::clang_visitChildren(c, visitor_fn::go, this);
	}
};
auto dummy = [](auto c){return true;};
template <typename Pred = bool(XCursor c)>
auto children(CXCursor parent, Pred pred = dummy) {
	vector<XCursor> ret;
	auto f = [&ret, pred](auto& c) {
		if (pred(c))
			ret.push_back(c);
		return CXChildVisit_Continue;
	};
	visitor_fn<decltype(f)> run(parent, f);
	return ret;
}

//! TODO Use range generator
auto genArgs(const XCursor& funC) {
	Function::Args args;
	using namespace xclang::views;
	CXCursorKind functionKinds[] = {CXCursor_Constructor, CXCursor_FunctionDecl, CXCursor_CXXMethod, CXCursor_FunctionTemplate};
	auto kind = funC.kind();
	assert(rng::contains(functionKinds, kind));
	if (kind == CXCursor_FunctionTemplate) {
//		return (vector<XCursor> const&) ::children(funC) // need either local copy or const ref to hold the temporary
//			| vs::filter([](auto kind){ return kind == CXCursor_ParmDecl;}, &XCursor::kind)
//			| vs::transform([](const XCursor& c){ return Function::Arg(c.type(), c.name()); })
//			| rng::to_vector;

//		auto ch = ::children(funC);
//		for (auto&& c : ch | vs::filter([](auto kind){ return kind == CXCursor_ParmDecl;}, &XCursor::kind)) {
		for (auto&& c : ::children(funC, [](auto c){return c.kind() == CXCursor_ParmDecl;})) {
			args.emplace_back(c.type(), c.spelling());
		}
	} else {
		for (auto&& c : argumenttCursors(funC)) {
			args.emplace_back(c.type(), c.spelling());
		}
	}
	return args;
}

[[maybe_unused]]
static char const* CXTemplateArgumentKind_Printable[] = {
  "CXTemplateArgumentKind_Null",
  "CXTemplateArgumentKind_Type",
  "CXTemplateArgumentKind_Declaration",
  "CXTemplateArgumentKind_NullPtr",
  "CXTemplateArgumentKind_Integral",
  "CXTemplateArgumentKind_Template",
  "CXTemplateArgumentKind_TemplateExpansion",
  "CXTemplateArgumentKind_Expression",
  "CXTemplateArgumentKind_Pack",
  /* Indicates an error case, preventing the kind from being deduced. */
  "CXTemplateArgumentKind_Invalid"
};

//! template args of a function decl representing a template specialization
[[maybe_unused]]
auto getTmplArgs(const XCursor& cursor) {
	auto n = clang_Cursor_getNumTemplateArguments(cursor);
	if (n >= 0)
		for (auto k = 0; k != n; ++k) {
			XType const& tmplArgType = clang_Cursor_getTemplateArgumentType(cursor, k);
			CXTemplateArgumentKind tmplArgKind = clang_Cursor_getTemplateArgumentKind(cursor, k);
			assert((unsigned int) tmplArgKind < sizeof(CXTemplateArgumentKind_Printable) / sizeof(*CXTemplateArgumentKind_Printable));
			auto kind = CXTemplateArgumentKind_Printable[(int) tmplArgKind];
			TraceX(kind, tmplArgType.name());
		}
}

}


Function::Function(const XCursor& cursor, const Container* p)
	: Container(cursor.data(), p)
	, retType(genResultType(cursor))
	, args(genArgs(cursor))
{
}

struct CXXMethod : Function {
	explicit CXXMethod(const XCursor& cursor, const Container* p = nullptr);
	string kindSpelling() const override { return "CXXMethod"; };
	Render& accept(Render& r) const override { return r.renderCXXMethod(*this); }
	string signature() const override { return Function::signature() + (isConst ? " const" : "") + ";";  }
	bool isConst;

};

CXXMethod::CXXMethod(const XCursor& cursor, const Container* p)
	: Function(cursor, p)
	, isConst(clang_CXXMethod_isConst(cursor))
{
	TraceX(signature());
}

struct Constructor : Function {
	explicit Constructor(const XCursor& cursor, const Container* p = nullptr) : Function(cursor, p) {}
	string kindSpelling() const override { return "Constructor"; };
	Render& accept(Render& r) const override { return r.renderConstructor(*this); }
};

struct Var : Node {
	explicit Var(const Entity& data, const Container* p = nullptr) : Node(data, p) {}
	string kindSpelling() const override { return "Variable"; }
	string signature() const override { return data.typeName + " " + name();  }
	Render& accept(Render& r) const override { return r.renderVar(*this); }
};

struct Field : Node {
	explicit Field(const Entity& data, const Container* p = nullptr) : Node(data, p) {}
	string kindSpelling() const override { return "Field"; };
	string signature() const override { return data.typeName + " " + name();  }
	Render& accept(Render& r) const override { return r.renderField(*this); }
};

struct Enum : Node {
	explicit Enum(const Entity& data, const Container* p = nullptr) : Node(data, p) {}
	string kindSpelling() const override { return "Eum"; };
	Render& accept(Render& r) const override { return r.renderEnum(*this); }
};

struct TmplArg : Node {
	explicit TmplArg(const Entity& n) : Node(n, nullptr) {}
//		string signature() const override { return data.name; };
};

struct FunctionTemplate : Function {
	explicit FunctionTemplate(const XCursor& cursor, const Container* p = nullptr);
	string kindSpelling() const override { return "FunctionTemplate"; };
	Render& accept(Render& r) const override { return r.renderFunctionTemplate(*this); }

	using TmplArgs = vector<TmplArg>;
	TmplArgs tmplArgs;
	string signature() const override;
};

namespace {

auto templateArgs(const XCursor& cursor) {
	vector<TmplArg> args;
	for (auto& x : ::children(cursor)) {
		switch (x.kind()){
			case CXCursor_TemplateTypeParameter:
			case CXCursor_TemplateTemplateParameter:
			case CXCursor_NonTypeTemplateParameter:
				args.emplace_back(x.data());
				break;
			default:
				break;
		}
	}
	return args;
}
}
FunctionTemplate::FunctionTemplate(const XCursor& cursor, const Container* p)
	: Function(cursor, p)
	, tmplArgs(templateArgs(cursor))
{
//	TraceX(signature());
}
string FunctionTemplate::signature() const {
//	return Function::signature();
	ostringstream buf;
	buf << retType << ' ';
	buf << name();
	buf << "<";
	for (int ix = 0; auto& arg : tmplArgs) {
		if (ix++) buf << ", ";
		if (arg.data.cursorKind == CXCursor_TemplateTypeParameter)
			buf << "class " << arg.name();
		else if (arg.data.cursorKind == CXCursor_NonTypeTemplateParameter)
			buf << arg.data.typeName << " " << arg.name();
		else if (arg.data.cursorKind == CXCursor_TemplateTemplateParameter)
			buf << "template <> class " << arg.name();
	}
	buf << ">";
	buf << '(';
	for (int ix = 0; auto& arg : args) {
		if (ix++) buf << ", ";
		buf << arg.type << ' ' << arg.name;
	}
	buf << ')';
	return buf.str();
}
/*
CINDEX_LINKAGE int 	clang_Cursor_getNumTemplateArguments (CXCursor C)
 	Returns the number of template args of a function decl representing a template specialization. More...

CINDEX_LINKAGE enum CXTemplateArgumentKind 	clang_Cursor_getTemplateArgumentKind (CXCursor C, unsigned I)
 	Retrieve the kind of the I'th template argument of the CXCursor C. More...

CINDEX_LINKAGE CXType 	clang_Cursor_getTemplateArgumentType (CXCursor C, unsigned I)
 	Retrieve a CXType representing the type of a TemplateArgument of a function decl representing a template specialization. More...

CINDEX_LINKAGE long long 	clang_Cursor_getTemplateArgumentValue (CXCursor C, unsigned I)
 	Retrieve the value of an Integral TemplateArgument (of a function decl representing a template specialization) as a signed long long. More...

CINDEX_LINKAGE unsigned long long 	clang_Cursor_getTemplateArgumentUnsignedValue (CXCursor C, unsigned I)
 	Retrieve the value of an Integral TemplateArgument (of a function decl representing a template specialization) as an unsigned long long. More...
*/
struct Namespace : public Container {
public:
	explicit Namespace(const Entity& data, const Container* p = nullptr) : Container(data, p) {
		log_trace << "Namespace ctor: " << data.name;
	}
	string kindSpelling() const override { return "Namespace"; };
	Render& accept(Render& r) const override { return r.renderNamespace(*this); }
};

class Registry {
public:
	Struct* getOrAdd(const Entity& data, const Container* p = nullptr) {
		auto [it, result] = registry_.try_emplace(data.usr, data, p);
		TraceX(result, registry_.size(), data.name, data.typeName);
		assert(it != registry_.end());
		return it->second.ptr;
	}
private:
	struct StructPtr {
		StructPtr(const Entity& data, const Container* p = nullptr);
		Struct* ptr;
	};
	unordered_map<string, StructPtr> registry_;
};

struct Struct : public Container {
public:
	explicit Struct(const Entity& data, const Container* p = nullptr) : Container(data, p) {}
//	string kindSpelling() const override { return "Struct"; };
	string name() const override { return data.typeName; };
	Render& accept(Render& r) const override { return r.renderStruct(*this); }

	static Registry registry;
};

struct ClassTemplate : public Container {
public:
	explicit ClassTemplate(const XCursor& cursor, const Container* p = nullptr);
	string kindSpelling() const override { return data.cursorKindS(); };
//	string name() const override { return data.typeName; };
	Render& accept(Render& r) const override { return r.renderClassTemplate(*this); }

	using TmplArgs = vector<TmplArg>;
	TmplArgs tmplArgs;
	XCursor cursor;
	string signature() const override;

	static Registry registry;
};

ClassTemplate::ClassTemplate(const XCursor& c, const Container* p)
	: Container(c.data(), p)
	, tmplArgs(templateArgs(c))
	, cursor(c)
{
}

string ClassTemplate::signature() const {
	ostringstream buf;
	buf << "template<";
	for (int ix = 0; auto& arg : tmplArgs) {
		if (ix++) buf << ", ";
		if (arg.data.cursorKind == CXCursor_TemplateTypeParameter)
			buf << "class " << arg.name();
		else if (arg.data.cursorKind == CXCursor_NonTypeTemplateParameter)
			buf << arg.data.typeName << " " << arg.name();
		else if (arg.data.cursorKind == CXCursor_TemplateTemplateParameter)
			buf << "template <> class " << arg.name();
	}
	buf << "> class ";
	buf << name();
	return buf.str();
}


Registry::StructPtr::StructPtr(const Entity& data, const Container* p) : ptr(new Struct(data, p)) {}
Registry Struct::registry;


struct Tree : public Container {
public:
	string kindSpelling() const override { return "Root"; };
	Render& accept(Render& r) const override { return r.renderTree(*this); }
};

Render& Render::renderTree(const Tree& x) { return renderContainer(x); }
Render& Render::renderNamespace(const Namespace& x) { return renderContainer(x); }
Render& Render::renderStruct(const Struct& x) { return renderContainer(x); }
Render& Render::renderClassTemplate(const ClassTemplate& x) { return renderContainer(x); }
Render& Render::renderFunction(const Function& x) { return renderContainer(x); }// { return renderNode(x); }
Render& Render::renderFunctionTemplate(const FunctionTemplate& x) { return renderContainer(x); }// { return renderNode(x); }
Render& Render::renderCXXMethod(const CXXMethod& x) { return renderFunction(x); }
Render& Render::renderConstructor(const Constructor& x) { return renderNode(x); }
Render& Render::renderVar(const Var& x) { return renderNode(x); }
Render& Render::renderField(const Field& x) { return renderNode(x); }
Render& Render::renderEnum(const Enum& x) { return renderNode(x); }

Json& Json::renderTree(const Tree& tree) {
	lines.emplace_back("{");
	for (auto& c : tree.children) {
		Indenter i(indent);
		c->accept(*this);
	}
	close();
	return *this;
}

Json& Json::renderNode(const Node& node) {
	renderNodeOpen(node);
	close();
	return *this;
}

Json& Json::renderContainer(const Container& container) {
	renderNodeOpen(container); // prolog
	for (auto& c : container.children) {
		Indenter i(indent);
		c->accept(*this);
	}
	close();
	return *this;
}

Json& Json::renderFunction(const Function& x) {
	renderNodeOpen(x);
	string prefix = string(indent * 4, ' ') + "  ";
//	lines.push_back( prefix + "." + node.name() + " : " + node.kindSpelling() );
	ostringstream buf_;
#define buf buf_ << prefix
	buf <<  quoted("args") << ": [\n";
	{
		Indenter i(indent);
		prefix = string(indent * 4, ' ');
		for (int i = 0; auto& arg : x.args) {
			buf << quoted(arg.type);
			if (++i < x.args.size()) { buf << ","; }
			buf << "\n";
		}
	}
	prefix = string(indent * 4, ' ') + "  ";
	buf << "]";
	lines.emplace_back( buf_.str() );
	close();
	return *this;
#undef buf
}


CXVisitorResult fieldsVisitor(CXCursor c, CXClientData client_data) {
	assert(client_data);
	const XCursor& cursor(c);
//	Container& parent(*reinterpret_cast<Container*>(client_data));
//	auto entity = cursor.data();
	switch (cursor.kind()) {
		default:
			TraceX(cursor, cursor.kindS(), clang_Cursor_isAnonymous(cursor), clang_Cursor_isAnonymousRecordDecl(cursor));
			const XCursor declCursor(clang_getTypeDeclaration(cursor.type()));
			TraceX(declCursor, declCursor.kindS());
			TraceX(clang_Cursor_isAnonymousRecordDecl(declCursor));
			TraceX(clang_Cursor_isAnonymous(declCursor));
	}
	return CXVisit_Continue;
}


class Visitor {
	struct Data {Visitor* v; Container* p;};
	static CXChildVisitResult go(CXCursor c, CXCursor p, CXClientData client_data) {
		auto [self, parent] = *reinterpret_cast<Data*>(client_data);
		return self->typesVisitor_(c, *parent);
	}
	CXChildVisitResult typesVisitor_(CXCursor c, Container& parent);
	string headerPath;
	bool verbose = false;
public:
	Visitor(string arg) : headerPath(arg) {}
	auto visitChildren(CXCursor c, CXClientData parent) try {
		Data data = {this, (Container*)parent};
		return ::clang_visitChildren(c, Visitor::go, &data);
	} catch (exception& e) {
		log_error << e.what();
		return 99u; // ???
	}
};

struct IncrIndent {
	IncrIndent() { indent++; }
	~IncrIndent() { --indent; }
	auto get() const { return string_view(buf, indent); }
//	operator const string_view() const { return get(); }
private:
	static char constexpr buf[] = ".........................................";
	static size_t indent;
};
size_t IncrIndent::indent = 0;

CXChildVisitResult Visitor::typesVisitor_(CXCursor c, Container& parent)
{
	if (headerPath != getContainingFile(c)) return CXChildVisit_Continue;
//	assert(client_data);
	const XCursor& cursor(c);
	IncrIndent _indent [[maybe_unused]];
//	cerr << _indent.get() << " " << cursor.spelling() << " : " << cursor.kindS() << endl;
	if (verbose) TraceX(cursor);
//	Container& parent(*reinterpret_cast<Container*>(client_data));

//	XCursor semanticParent = clang_getCursorSemanticParent(cursor); // Field need this to calculate offset
//	// Consider that: a typedef is a parent, but not the semantic parent
//	if (semanticParent.usr() != parent.usr()) {
//		log_trace << "P: this{" << cursor << "}; parent{" << semanticParent << "}; container{" << parent << "}";
//	}

	auto entity = cursor.data();
	switch (cursor.kind()) {
		case CXCursor_Namespace:
			TraceX("Namespace", clang_Cursor_isAnonymous(cursor));
			if (cursor.spelling().empty()) break; // skip anonymous namespace
			if (Node* x = parent.get(entity.usr)) {	 // namespace object already exists - reuse it
				visitChildren(cursor, x);
			} else if ((x = new Namespace(cursor.data(), &parent))) {  // create a new one
				visitChildren(cursor, x);
				parent.add(x);
			}
			break;

		case CXCursor_TypedefDecl:
			TraceX("TypedefDecl", clang_Cursor_isAnonymous(cursor), cursor.spelling(), cursor.type());
			if (auto x = new Container(cursor.data(), &parent)) {
				visitChildren(cursor, x);
				parent.add(x);
			}
			break;

		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
		case CXCursor_UnionDecl: {
			TraceX("Class/Struct/UnionDecl", clang_Cursor_isAnonymous(cursor), cursor.spelling(), cursor.type(), parent);
			if (Struct* x = Struct::registry.getOrAdd(cursor.data(), &parent)) {
//			if (auto x = new Struct(cursor.data(), parent)) {
				visitChildren(cursor, x);
				parent.add(x);
//				auto align = clang_Type_getAlignOf(cursor.type());
//				auto size = clang_Type_getSizeOf(cursor.type());
//				TraceX(cursor, size, align);
//				TraceX(*x);

//				clang_Type_visitFields(cursor.type(), fieldsVisitor, x);
			}
			break;
		}

		case CXCursor_ClassTemplate:
			if (auto x = new ClassTemplate(cursor, &parent)) {
				visitChildren(cursor, x);
				parent.add(x);
			}
			break;

		case CXCursor_Constructor:
			if (auto x = new Constructor(cursor, &parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_CXXMethod:
			if (auto x = new CXXMethod(cursor, &parent)) {
//				visitChildren(cursor, x);
				if (!x->name().starts_with("__"sv))
				parent.add(x);
			}
			break;

		case CXCursor_FunctionDecl:
			if (auto x = new Function(cursor, &parent)) {
//				visitChildren(cursor, x);
				if (!x->name().starts_with("__"sv))
				parent.add(x);
			}
			break;

		case CXCursor_FunctionTemplate:
			log_trace << "<< CXCursor_FunctionTemplate: " << cursor;
			if (auto x = new FunctionTemplate(cursor, &parent)) {
				if (!x->name().starts_with("__"sv))
					parent.add(x);
			}
			log_trace << ">>: ";
			break;

		case CXCursor_VarDecl:
			if (auto x = new Var(cursor.data(), &parent)) {
//				XType canonT = clang_getCanonicalType(cursor.type());
//				auto sizeOfT = clang_Type_getSizeOf(cursor.type());
//				if (canonT.kind == CXType_Vector) {
//					XType elemT = clang_getElementType(canonT);
//					TraceX(clang_getArraySize(canonT));
//					TraceX(x->name(), sizeOfT);
//					TraceX(elemT.name(), elemT.kindS());
//					Trace3(x->name(), x->data.cursorKindS(), x->data.typeKindS());
//					Trace3(x->name(), canonT.name(), canonT.kindS());
//				}
//				auto align = clang_Type_getAlignOf(cursor.type());
//				TraceX(x->name(), sizeOfT, align, cursor.typeName());
				parent.add(x);
			}
			break;

		case CXCursor_FieldDecl:
			if (auto x = new Field(cursor.data(), &parent)) {
				parent.add(x);
			}
			break;

		case CXCursor_EnumDecl:
			if (auto x = new Enum(cursor.data(), &parent)) {
				parent.add(x);
			}
			break;

		default:
			if (Node* node = new Node(cursor.data(), &parent)) {
				log_trace << "default> " << cursor;
//				TraceX("default", cursor.data().cursorKindS(),  *node);
			}
			return CXChildVisit_Recurse;
	}
	return CXChildVisit_Continue;
}


void  parseTypes(
		const std::vector<std::string>& files,
		const Args& args //, const Options&	 options
		)
{
	for (const auto& file : files)
	{
		CXIndex index =	 clang_createIndex(0, 0);
		CXTranslationUnit unit = parse(index, file, args);

		auto cursor = clang_getTranslationUnitCursor(unit);

		Tree tree;
		Visitor v(file);
		v.visitChildren(cursor, &tree);

		cout << tree << endl;

		Json json;
		tree.accept(json);
	//	return os << log << endl;
		cerr << json << endl;

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}
}
