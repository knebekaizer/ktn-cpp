#include "parser.function.hpp"

#include "parser.util.hpp"

#include "serializer.function.hpp"
#include "trace.h"

using namespace reflang;
using namespace std;

Function parser::getFunction(CXCursor cursor)
{
	Function f(getFile(cursor), getFullName(cursor),
		parser::convert(clang_Cursor_getMangling(cursor)));
	auto type = clang_getCursorType(cursor);

	f.name = parser::convert(clang_getCursorSpelling(cursor));
	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i)
	{
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		NamedObject arg;
		arg.name = parser::convert(clang_getCursorSpelling(arg_cursor));
		if (arg.name.empty())
		{
			arg.name = "nameless";
		}
		auto arg_type = clang_getArgType(type, i);
		arg.type = parser::getName(arg_type);
		f.arguments.push_back(arg);
	}

	f.returnType = parser::getName(clang_getResultType(type));
    log_trace << "F> " << f;
	serializer::genDefinition(std::cout, f);

	return f;
}

