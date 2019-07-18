#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.class.hpp"
#include "parser.enum.hpp"
#include "parser.function.hpp"
#include "parser.util.hpp"

#include "generator.h"

#include "trace.h"

using namespace std;
using namespace ktn;

namespace
{

CXTranslationUnit parse(
		CXIndex& index, const string& file, int argc, char** argv)
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
		//	exit(-1);
		}
	}

	return unit;
}

struct GetTypesStruct
{
	vector<unique_ptr<TypeBase>>* types;
	const ktn::Options* options;
};

CXChildVisitResult getTypesVisitor(
		CXCursor cursor, CXCursor parent, CXClientData client_data)
{

	auto* data = reinterpret_cast<GetTypesStruct*>(client_data);

	// Awfull. FIXME registry
	static unordered_set<string> types_registry;   // classes & structs
	static unordered_set<string> symbols_registry; // var & functions
	static WildCard path(data->options->path_filter);

	if (!path.match(ktn::getFile(cursor))) return CXChildVisit_Recurse;  // filter out "external" types

	std::unique_ptr<TypeBase> type;
	switch (clang_getCursorKind(cursor)) {
		case CXCursor_EnumDecl:
			type = std::make_unique<Enum>(ktn::buildEnum(cursor));
			break;
		case CXCursor_ClassDecl:
		case CXCursor_StructDecl:
			if (!types_registry.insert(buildFullName(cursor)).second) break;  // skip dups
			type = std::make_unique<Class>(ktn::buildClass(cursor));
			break;
		case CXCursor_FunctionDecl:
			if (isOperatorFunction(cursor)) break;
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
}
		default:
			break;
	}

	if (type) {
		const string& name = type->fullName();
		if (type
		    && !name.empty()
		    && ktn::isRecursivelyPublic(cursor)
		    && name.back() != ':'
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

vector<string> ktn::getSupportedTypeNames(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options)
{
	auto types = getTypes(files, argc, argv, options);

	vector<string> names;
	names.reserve(types.size());
	for (const auto& type : types)
	{
		names.push_back(type->fullName());
	}
	return names;
}

vector<unique_ptr<TypeBase>> ktn::  getTypes(
		const std::vector<std::string>& files,
		int argc, char** argv,
		const Options& options,
		WrapperGenerator* gen)
{
//	for (int k=0; k!= argc; ++k) log_trace << argv[k];
	vector<unique_ptr<TypeBase>> results;
	auto last = 0;
	for (const auto& file : files)
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = parse(index, file, argc, argv);

		auto cursor = clang_getTranslationUnitCursor(unit);

		GetTypesStruct data = { &results, &options };
		clang_visitChildren(cursor, getTypesVisitor, &data);

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	//	log_debug << "Loaded " << results.size() << " types";
		if (gen) gen->genWrappers(results.begin() + last, results.end());
	}
	return results;
}
