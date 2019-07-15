#include "parser.function.hpp"

#include "parser.util.hpp"
#include "types.hpp"

#include "serializer.function.hpp"
#include "trace.h"

using namespace reflang;
using namespace std;
/*
namespace std {
std::ostream& operator<<(std::ostream& os, const reflang::Function &f);
}
*/
Function parser::getFunction(CXCursor cursor)
{
	auto type = clang_getCursorType(cursor);

	Function f(parser::getFile(cursor), parser::getFullName(cursor));
	f.name = parser::convert(clang_getCursorSpelling(cursor));
	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i) {
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		auto name = parser::convert(clang_getCursorSpelling(arg_cursor));
		if (name.empty()) {
			name = "_arg" + std::to_string(i); // TODO make uniq
		}
		auto arg_type = clang_getArgType(type, i);
		auto type = parser::getName(arg_type);
		Function::Argument arg(type, name);  // TODO isRef, isConst
		log_info << "Arg " << clang_getTypeSpelling(arg_type) << " " << arg.name << "> " << clang_getTypeKindSpelling(arg_type.kind) << ": "
		         << clang_getTypeSpelling(clang_getPointeeType(arg_type)) << ": " << clang_getTypeKindSpelling(clang_getPointeeType(arg_type).kind);
		f.arguments.push_back(arg);
	}

	f.returnType = parser::getName(clang_getResultType(type));
	log_trace << f << " # " << clang_Cursor_getMangling(cursor);
	genDefinition((cout << endl), f);
	return f;
}

