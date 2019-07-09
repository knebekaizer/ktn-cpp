#include "trace.h"

#include <clang-c/Index.h>

#include <iostream>
#include <string>

using namespace std;

std::string getCursorKindName(CXCursorKind cursorKind) {
    CXString kindName = clang_getCursorKindSpelling(cursorKind);
    std::string result = clang_getCString(kindName);

    clang_disposeString(kindName);
    return result;
}

std::string getCursorSpelling(CXCursor cursor) {
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
    std::string result = clang_getCString(cursorSpelling);

    clang_disposeString(cursorSpelling);
    return result;
}

using Cursor = CXCursor;

class Member {
public:
    string details(Cursor c) {
        auto access = clang_getCXXAccessSpecifier(c);
    }
};

CXChildVisitResult visitor(CXCursor cursor, CXCursor /* parent */, CXClientData clientData) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    if (clang_Location_isFromMainFile(location) == 0)
        return CXChildVisit_Continue;

    CXCursorKind cursorKind = clang_getCursorKind(cursor);

    unsigned int curLevel = *(reinterpret_cast<unsigned int *>( clientData ));
    unsigned int nextLevel = curLevel + 1;

    std::cout << std::string(curLevel, '.') << getCursorKindName(
            cursorKind) << " (" << getCursorSpelling(cursor) << ")\n";

    clang_visitChildren(cursor,
                        visitor,
                        &nextLevel);

    return CXChildVisit_Continue;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        log_error << "[Missed parameter] Usage: " << argv[0] << " <file>";
        return -1;
    }
    log_info << "Processing " << argv[1];

    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu = // clang_createTranslationUnit( index, argv[1] );
            clang_parseTranslationUnit(
                    index,
                    argv[1], nullptr, 0,
                    nullptr, 0,
                    CXTranslationUnit_None);
    if (!tu) {
        log_error << "clang_parseTranslationUnit failed";
        return -1;
    }

    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

    unsigned int treeLevel = 0;

    clang_visitChildren(rootCursor, visitor, &treeLevel);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
