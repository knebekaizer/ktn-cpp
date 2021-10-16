//
// Created by Vladimir Ivanov on 18/09/2019.
//
#include "parser_tree.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <clang-c/Index.h>


#include "trace.h"

#pragma clang diagnostic ignored "-Wunused-function"

using namespace std;
//using namespace ktn;


int parse(
        CXIndex& index, const string& file, const Args& args)
{
    /*
    CXTranslationUnit unit = clang_parseTranslationUnit(
            index,
            file.c_str(), &args[0], args.size(),
            nullptr, 0,
            CXTranslationUnit_None);
    if (unit == nullptr)
    {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        //	exit(-1);
    }
*/
    CXTranslationUnit  unit;
    auto err = clang_parseTranslationUnit2(
            index,
            file.c_str(), args, args.size(),
            nullptr, 0,
            0, //CXTranslationUnit_DetailedPreprocessingRecord, // 0
            &unit
            );
    log_error << "err_code = " << (int)err;

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

    return (int)err;
}

int main() {
    Args args = {
        //   "-framework", "ARKit", // "CryptoTokenKit", //
            "-isystem", "Volumes/vdi/work/src/llvm-apple/clang-llvm-apple/lib/clang/9.0.0/include/",
            "-B/Volumes/vdi/.konan/dependencies/target-toolchain-10-macos_x64/usr/bin",
            "-fno-stack-protector",
            "-stdlib=libc++",
            "-arch", "armv7",
            "--sysroot=/Volumes/vdi/.konan/dependencies/target-sysroot-10-ios_arm64",
            "-miphoneos-version-min=9.0",
            "-fobjc-arc",
            "-fmodules"
          //  , "-fmodules-cache-path=./clangModulesCache"
    };
/*
     {       "-framework", "Accelerate",
            -isystem, /Volumes/vdi/.konan/dependencies/clang+llvm-8.0.1-x86_64-apple-darwin/lib/clang/10.0.0/include,
            -B/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin,
            -fno-stack-protector,
            --sysroot=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk,
            -mmacosx-version-min=10.11,
            -fobjc-arc, -fmodules
            , -fmodules-cache-path=/var/folders/d4/gzt84b8n2b74skhyh0v7mrt40000gp/T/ModuleCache16941998183889473399.tmp
    };
*/
    CXIndex index = clang_createIndex(0, 1);
//    CXTranslationUnit unit =
    auto err = parse(index, "x.m", args);

    return err;
}