#include "parser.function.hpp"

#include "parser.util.hpp"
#include "types.hpp"

#include <memory>

#include "trace.h"

using namespace std;
using namespace ktn;


Function ktn::buildFunction(CXCursor cursor)
{
	auto type = clang_getCursorType(cursor);

	Function f(ktn::getFile(cursor), ktn::buildFullName(cursor), clang_CXXMethod_isConst(cursor));
	f.short_name = ktn::convertAndDispose(clang_getCursorSpelling(cursor));

	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i) {
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		auto arg_name = convertAndDispose(clang_getCursorSpelling(arg_cursor));
		if (arg_name.empty()) {
			arg_name = "_arg" + std::to_string(i); // TODO make uniq
		}

		Function::Argument arg(arg_name, buildCxxType(arg_cursor));
		TraceX(arg.dump());
		f.arguments.push_back(arg);
	}

	f.returnType = buildCxxType(clang_getResultType(type));
	log_trace << f << " # " << clang_Cursor_getMangling(cursor);
	return f;
}

void Function::setReceiver(CxxType&& thiz)
{
	receiver = std::make_unique<CxxType>(thiz);
}
