//
// Created by Володя on 2019-08-21.
//

#ifndef KTN_CPP_UTILS_H
#define KTN_CPP_UTILS_H

#include <clang-c/Index.h>

std::ostream& operator<<(std::ostream& os, CXString&& s);
std::ostream& operator<<(std::ostream& os, const CXType& t);


std::string convertAndDispose(const CXString& s);
std::string buildFullName(CXCursor cursor);
std::string getTypeSpelling(const CXType& type);
std::string getFile(const CXCursor& cursor);

bool isOperatorFunction(CXCursor cursor);

#endif //KTN_CPP_UTILS_H
