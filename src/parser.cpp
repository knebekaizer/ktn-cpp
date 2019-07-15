#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.class.hpp"
#include "parser.enum.hpp"
#include "parser.function.hpp"
#include "parser.util.hpp"

#include "serializer.function.hpp"

#include "trace.h"

using namespace reflang;
using namespace std;

namespace
{

void printTypeInfo(CXCursor cursor) {
	auto t = clang_getCursorType (cursor);
	TraceX(t);

	ostringstream tmp;

	auto derefCount = 0;
	while ((t = clang_getPointeeType(clang_getCanonicalType(t))).kind != CXType_Invalid) {
		++derefCount;
		log_trace << clang_getTypeSpelling(t) << string(derefCount, '*');
		TraceX(t);
		TraceX(clang_getCanonicalType(t));
	}

/*
//	clang_getTypedefDeclUnderlyingType (CXCursor C)
//	clang_getEnumDeclIntegerType (CXCursor C)
	clang_isPODType (CXType T)
	clang_Type_getNamedType (CXType T)
	clang_Type_getCXXRefQualifier (CXType T)
	log_info << "Var: " <<  clang_getCursorSpelling(cursor) << "> "
	         << "\n type: " <<  t
	         << "\n canonical: " << clang_getCanonicalType(t)
	         << "\n pointee: " << clang_getPointeeType(t)
	         << "\n elemKind: " << clang_getElementType(t) ;
*/
}

CXTranslationUnit Parse(
		CXIndex& index, const string& file, int argc, char* argv[])
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
			exit(-1);
		}
	}

	return unit;
}

struct GetTypesStruct
{
	vector<unique_ptr<TypeBase>>* types;
	const parser::Options* options;
};

CXChildVisitResult GetTypesVisitor(
		CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	auto* data = reinterpret_cast<GetTypesStruct*>(client_data);
	std::unique_ptr<TypeBase> type;
	switch (clang_getCursorKind(cursor))
	{
		case CXCursor_EnumDecl:
			type = std::make_unique<Enum>(parser::getEnum(cursor));
			break;
		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
			type = std::make_unique<Class>(parser::getClass(cursor));
			break;
		case CXCursor_FunctionDecl:
			type = std::make_unique<Function>(parser::buildFunction(cursor));
			break;
		case CXCursor_VarDecl:
{
auto t = clang_getCursorType (cursor);
log_info << "Var: " <<  clang_getCursorSpelling(cursor) << "> "
     << "\n type: " <<  t
	 << "\n canonical: " << clang_getCanonicalType(t)
	 << "\n pointee: " << clang_getPointeeType(t)
     << "\n elemKind: " << clang_getElementType(t) ;
printTypeInfo(cursor);
}

		default:
			break;
	}

	const string& name = type->getFullName();
	if (type
			&& !name.empty()
			&& parser::isRecursivelyPublic(cursor)
			&& !(name.back() == ':')
			&& regex_match(name, data->options->include)
			&& !regex_match(name, data->options->exclude))
	{
		data->types->push_back(std::move(type));
	}

	return CXChildVisit_Recurse;
}

}  // namespace

vector<string> parser::GetSupportedTypeNames(
		const std::vector<std::string>& files,
		int argc, char* argv[],
		const Options& options)
{
	auto types = GetTypes(files, argc, argv, options);

	vector<string> names;
	names.reserve(types.size());
	for (const auto& type : types)
	{
		names.push_back(type->getFullName());
	}
	return names;
}

vector<unique_ptr<TypeBase>> parser::  GetTypes(
		const std::vector<std::string>& files,
		int argc, char* argv[],
		const Options& options)
{
//	for (int k=0; k!= argc; ++k) log_trace << argv[k];
	vector<unique_ptr<TypeBase>> results;
	for (const auto& file : files)
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = Parse(index, file, argc, argv);

		auto cursor = clang_getTranslationUnitCursor(unit);

		GetTypesStruct data = { &results, &options };
		clang_visitChildren(cursor, GetTypesVisitor, &data);

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}
	return results;
}
