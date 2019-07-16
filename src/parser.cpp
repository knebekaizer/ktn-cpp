#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.class.hpp"
#include "parser.enum.hpp"
#include "parser.function.hpp"
#include "parser.util.hpp"

#include "trace.h"

using namespace std;
using namespace ktn;

namespace
{

void printTypeInfo(CXCursor cursor) {
	auto t = clang_getCursorType (cursor);
	TraceX(t);

	ostringstream tmp;

/*
printTypeInfo> t = auto.Auto
printTypeInfo> clang_getCanonicalType(t) = auto.Auto
printTypeInfo> auto*******************************************
 ... infinite loop
	auto derefCount = 0;
	while ((t = clang_getPointeeType(clang_getCanonicalType(t))).kind != CXType_Invalid) {
		++derefCount;
		log_trace << clang_getTypeSpelling(t) << string(derefCount, '*');
		TraceX(t);
		TraceX(clang_getCanonicalType(t));
	}
*/
/*
//	clang_getTypedefDeclUnderlyingType (CXCursor C)
//	clang_getEnumDeclIntegerType (CXCursor C)
	clang_isPODType (CXType T)
	clang_Type_getNamedType (CXType T)
	clang_Type_getCXXRefQualifier (CXType T)
	log_trace << "Var: " <<  clang_getCursorSpelling(cursor) << "> "
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
	const ktn::Options* options;
};

CXChildVisitResult GetTypesVisitor(
		CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	// Awfull. FIXME registry
	static unordered_set<string> types_registry;   // classes & structs
	static unordered_set<string> symbols_registry; // var & functions

	auto* data = reinterpret_cast<GetTypesStruct*>(client_data);
	std::unique_ptr<TypeBase> type;
	switch (clang_getCursorKind(cursor)) {
		case CXCursor_EnumDecl:
			type = std::make_unique<Enum>(ktn::getEnum(cursor));
			break;
		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
			if (!types_registry.insert(getFullName(cursor)).second) break;  // skip dups
			type = std::make_unique<Class>(ktn::getClass(cursor));
			break;
		case CXCursor_FunctionDecl:
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
printTypeInfo(cursor);
}

		default:
			break;
	}

	WildCard path(data->options->path_filter);
	if (type && !path.match(type->getFile())) {
//		log_info << "Rejected by path dismatch: " << type->getFullName();
//log_info << type->getFullName() << " : " << type->getFile();
	}
	if (type) {
		const string& name = type->getFullName();
		if (type
		    && !name.empty()
		    && ktn::isRecursivelyPublic(cursor)
		    && !(name.back() == ':')
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

vector<string> ktn::GetSupportedTypeNames(
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

vector<unique_ptr<TypeBase>> ktn::  GetTypes(
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
		log_debug << "Loaded " << results.size() << " types";
	}
	return results;
}
