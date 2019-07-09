#include "parser.enum.hpp"

#include "parser.util.hpp"

using namespace reflang;
using namespace std;

namespace
{
	CXChildVisitResult VisitEnum(
			CXCursor cursor, CXCursor parent, CXClientData client_data)
	{
		if (clang_getCursorKind(cursor) == CXCursor_EnumConstantDecl)
		{
			string name = parser::convert(clang_getCursorSpelling(cursor));
			int value = static_cast<int>(clang_getEnumConstantDeclValue(cursor));
			reinterpret_cast<Enum::Values*>(client_data)->emplace_back(
					name, value);
		}
		return CXChildVisit_Continue;
	}

	Enum::Values GetEnumValues(const CXCursor& cursor)
	{
		Enum::Values result;

		clang_visitChildren(cursor, VisitEnum, &result);

		return result;
	}
}

Enum parser::getEnum(CXCursor cursor)
{
	Enum e(getFile(cursor), getFullName(cursor));
	e.values = GetEnumValues(cursor);
	return e;
}
