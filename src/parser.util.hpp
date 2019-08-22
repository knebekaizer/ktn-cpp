#ifndef REFLANG_PARSER_UTIL_HPP
#define REFLANG_PARSER_UTIL_HPP

#include "types.hpp"

#include <string>
#include <clang-c/Index.h>

#include "trace.h"

namespace ktn {

bool isRecursivelyPublic(CXCursor cursor);
//bool isRefType(const CXType& type);


/*
 * Rationale: why not CxxType(CxType) constructor?
 * - Constructor is merely to _construct_ object from its details
 * - Here we need a _conversion_ from one domain (clang) to another (KTN)
 * - For the sake of _low coupling_ it's better to keep domains isolated, while this only function depends on both
 * (is it Dependency Inversion?)
 */
CxxType buildCxxType(CXType type);

CxxType buildCxxType(CXCursor cursor);

std::string simpleMangling(std::string s, const char* prefix = "K2N_");


class WildCard {
public:
	explicit WildCard(std::string wild) : wildcard_(move(wild)) {}
	bool match(const std::string& s) const;
private:
	const std::string wildcard_;
};


} // namespace ktn



#endif //REFLANG_PARSER_UTIL_HPP

