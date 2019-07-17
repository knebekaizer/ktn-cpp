#include "parser.function.hpp"

#include "parser.util.hpp"
#include "types.hpp"

#include <memory>

#include "trace.h"

using namespace std;
using namespace ktn;
/*
namespace std {
std::ostream& operator<<(std::ostream& os, const Function &f);
}
*/
template <typename T> T convertTo(CXType t);

/*
 * Rationale: why not CxxType(CxType) constructor?
 * - Constructor is merely to _construct_ object from its details
 * - Here we need a _conversion_ from one domain (clang) to another (KTN)
 * - For the sake of _low coupling_ it's better to keep domains isolated, while this only function depends on both
 * (is it Dependency Inversion?)
 */
template <> CxxType convertTo<CxxType>(CXType t) {
	return CxxType(getTypeSpelling(t), isRefType(t));
}
CxxType buildCxxType(CXType type) {
	return CxxType(getTypeSpelling(type), isRefType(type));
}
CxxType buildCxxType(CXCursor cursor) {
	assert(clang_isDeclaration(clang_getCursorKind(cursor))); // caller is responsible to use in the proper context only!
	return buildCxxType(clang_getCursorType(cursor));
}


Function ktn::buildFunction(CXCursor cursor)
{
	auto type = clang_getCursorType(cursor);

	Function f(ktn::getFile(cursor), ktn::buildFullName(cursor), clang_CXXMethod_isConst(cursor));
	f.name = ktn::convertAndDispose(clang_getCursorSpelling(cursor));
	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i) {
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		TraceX(buildFullName(arg_cursor));
		auto arg_name = convertAndDispose(clang_getCursorSpelling(arg_cursor));
		if (arg_name.empty()) {
			arg_name = "_arg" + std::to_string(i); // TODO make uniq
		}
		auto arg_type = clang_getArgType(type, i);
		auto type_name = ktn::getTypeSpelling(arg_type);
		Function::Argument arg(arg_name, buildCxxType(arg_cursor));
		log_trace << "Arg " << arg.name << " : " <<  arg_type << " isPointerTo "
		         << clang_getPointeeType(arg_type);
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
