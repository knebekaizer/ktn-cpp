#ifndef REFLANG_PARSER_UTIL_HPP
#define REFLANG_PARSER_UTIL_HPP

#include <string>

#include <clang-c/Index.h>
#include "trace.h"

namespace ktn {

std::string convertAndDispose(const CXString& s);
std::string getFullName(CXCursor cursor);
std::string getName(const CXType& type);
std::string getFile(const CXCursor& cursor);

bool isRecursivelyPublic(CXCursor cursor);
bool isReference(const CXType& type);


class WildCard {
public:
	explicit WildCard(std::string wild) : wildcard_(move(wild)) {}
	bool match(const std::string& s) const;
private:
	const std::string wildcard_;
};

}


std::ostream& operator<<(std::ostream& os, CXString&& s);

std::ostream& operator<<(std::ostream& os, const CXType& t);


#endif //REFLANG_PARSER_UTIL_HPP

