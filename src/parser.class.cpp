#include "parser.class.hpp"

#include "parser.util.hpp"

#include "trace.h"

using namespace reflang;
using namespace std;

namespace {
Function getMethodFromCursor(CXCursor cursor)
{
	auto type = clang_getCursorType(cursor);

	Function f(
			parser::getFile(cursor), parser::getFullName(cursor),
			parser::convert(clang_Cursor_getMangling(cursor)));
	f.name = parser::convert(clang_getCursorSpelling(cursor));
	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i) {
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		NamedObject arg;
		arg.name = parser::convert(
				clang_getCursorSpelling(arg_cursor));
		if (arg.name.empty()) {
			arg.name = "nameless";
		}
		auto arg_type = clang_getArgType(type, i);
		arg.type = parser::getName(arg_type);
log_info << "Arg " << clang_getTypeSpelling(arg_type) << " " << arg.name << "> " << clang_getTypeKindSpelling(arg_type.kind) << ": "
<< clang_getTypeSpelling(clang_getPointeeType(arg_type)) << ": " << clang_getTypeKindSpelling(clang_getPointeeType(arg_type).kind);
		f.arguments.push_back(arg);
	}

	f.returnType = parser::getName(clang_getResultType(type));
	log_trace << f  << " # " << clang_Cursor_getMangling(cursor);
	return f;
}

NamedObject getFieldFromCursor(CXCursor cursor)
{
	NamedObject field;
	field.name = parser::convert(clang_getCursorSpelling(cursor));
	field.type = parser::getName(clang_getCursorType(cursor));
	log_trace << field  << " # " << move(clang_Cursor_getMangling(cursor));
	return field;
}



CXChildVisitResult visitClass(
		CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	auto *c = reinterpret_cast<Class *>(client_data);
	if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic) {
		switch (clang_getCursorKind(cursor)) {
			case CXCursor_Constructor:
			//	TraceX(clang_CXXConstructor_isConvertingConstructor(cursor));
			//	TraceX(clang_CXXConstructor_isCopyConstructor(cursor));
			//	TraceX(clang_CXXConstructor_isDefaultConstructor(cursor));
			//	TraceX(clang_CXXConstructor_isMoveConstructor(cursor));
				c->ctors.push_back(getMethodFromCursor(cursor));
				break;
			case CXCursor_Destructor:
			//	c->dtor = getMethodFromCursor(cursor);
				break;
			case CXCursor_CXXMethod:
				if (clang_CXXMethod_isStatic(cursor) != 0) {
					c->staticMethods.push_back(getMethodFromCursor(cursor));
				} else {
					c->methods.push_back(getMethodFromCursor(cursor));
				}
				break;
			case CXCursor_FieldDecl:
				c->fields.push_back(getFieldFromCursor(cursor));
{
auto t = clang_getCursorType (cursor);
log_info << "Field " << clang_getCursorSpelling(cursor) << "> "
		 << clang_getTypeSpelling(t) << ": " << clang_getTypeKindSpelling(t.kind) << ": "
         << clang_getTypeSpelling(clang_getPointeeType(t)) << ": " << clang_getTypeKindSpelling(clang_getPointeeType(t).kind);
}
				break;
			case CXCursor_VarDecl:
				c->staticFields.push_back(getFieldFromCursor(cursor));
{
auto t = clang_getCursorType (cursor);
log_info << "Static field " << clang_getCursorSpelling(cursor) << "> "
         << clang_getTypeSpelling(t) << ": " << clang_getTypeKindSpelling(t.kind) << ": "
         << clang_getTypeSpelling(clang_getPointeeType(t)) << ": " << clang_getTypeKindSpelling(clang_getPointeeType(t).kind);
}
				break;
			default:
				break;
		}
	}
	return CXChildVisit_Continue;
}
}

Class parser::getClass(CXCursor cursor)
{
	Class c(getFile(cursor), getFullName(cursor));
	clang_visitChildren(cursor, visitClass, &c);
	log_trace << "C> " << c << " # " << clang_Cursor_getMangling(cursor);

	return c;
}


/*

log_trace << clang_getTypeKindSpelling (cursor.)

CXCursor_Constructor                   = 24,
CXCursor_Destructor                    = 25,
CXCursor_ConversionFunction            = 26,

clang_CXXConstructor_isConvertingConstructor(CXCursor C);
clang_CXXConstructor_isCopyConstructor(CXCursor C);
clang_CXXConstructor_isDefaultConstructor(CXCursor C);
clang_CXXConstructor_isMoveConstructor(CXCursor C);

CINDEX_LINKAGE unsigned clang_CXXMethod_isStatic(CXCursor C);
CINDEX_LINKAGE unsigned clang_CXXMethod_isVirtual(CXCursor C);
CINDEX_LINKAGE unsigned clang_CXXRecord_isAbstract(CXCursor C);

CINDEX_LINKAGE unsigned clang_CXXMethod_isConst(CXCursor C);


CINDEX_LINKAGE CXType 	clang_getCanonicalType (CXType T)
 	Return the canonical type for a CXType. More...

CINDEX_LINKAGE CXString 	clang_getTypedefName (CXType CT)
 	Returns the typedef name of the given type. More...

CINDEX_LINKAGE CXType 	clang_getPointeeType (CXType T)
 	For pointer types, returns the type of the pointee. More...

 CINDEX_LINKAGE CXString 	clang_getTypeKindSpelling (enum CXTypeKind K)
 	Retrieve the spelling of a given CXTypeKind. More...

CINDEX_LINKAGE unsigned 	clang_isPODType (CXType T)
 	Return 1 if the CXType is a POD (plain old data) type, and 0 otherwise. More...


 CINDEX_LINKAGE long long 	clang_Type_getSizeOf (CXType T)
 	Return the size of a type in bytes as per C++[expr.sizeof] standard. More...

CINDEX_LINKAGE long long 	clang_Type_getOffsetOf (CXType T, const char *S)
 	Return the offset of a field named S in a record of type T in bits as it would be returned by offsetof as per C++11[18.2p4]. More...

CINDEX_LINKAGE long long 	clang_Cursor_getOffsetOfField (CXCursor C)

 */