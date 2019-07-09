#include "parser.class.hpp"

#include "parser.util.hpp"

#include "trace.h"

using namespace reflang;
using namespace std;

namespace
{
	Function GetMethodFromCursor(CXCursor cursor)
	{
		auto type = clang_getCursorType(cursor);

		Function f(
                parser::getFile(cursor), parser::getFullName(cursor));
		f.name = parser::convert(clang_getCursorSpelling(cursor));
		int num_args = clang_Cursor_getNumArguments(cursor);
		for (int i = 0; i < num_args; ++i)
		{
			auto arg_cursor = clang_Cursor_getArgument(cursor, i);
			NamedObject arg;
			arg.name = parser::convert(
                    clang_getCursorSpelling(arg_cursor));
			if (arg.name.empty())
			{
				arg.name = "nameless";
			}
			auto arg_type = clang_getArgType(type, i);
			arg.type = parser::getName(arg_type);
			f.arguments.push_back(arg);
		}

		f.returnType = parser::getName(clang_getResultType(type));
		return f;
	}

	NamedObject GetFieldFromCursor(CXCursor cursor)
	{
		NamedObject field;
		field.name = parser::convert(clang_getCursorSpelling(cursor));
		field.type = parser::getName(clang_getCursorType(cursor));
		return field;
	}

	CXChildVisitResult VisitClass(
			CXCursor cursor, CXCursor parent, CXClientData client_data)
	{
		auto* c = reinterpret_cast<Class*>(client_data);
		if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic)
		{
			switch (clang_getCursorKind(cursor))
			{
			case CXCursor_CXXMethod:
				if (clang_CXXMethod_isStatic(cursor) != 0)
				{
					c->staticMethods.push_back(GetMethodFromCursor(cursor));
				}
				else
				{
					c->methods.push_back(GetMethodFromCursor(cursor));
				}
				break;
			case CXCursor_FieldDecl:
				c->fields.push_back(GetFieldFromCursor(cursor));
				break;
			case CXCursor_VarDecl:
				c->staticFields.push_back(GetFieldFromCursor(cursor));
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
	clang_visitChildren(cursor, VisitClass, &c);
	log_trace << c;
	return c;
}
