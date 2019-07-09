	cc src/spdlog.cpp -Iinclude -std=c++14 -stdlib=libc++ -DSPDLOG_COMPILED_LIB -lc++ -c
the same as

	clang++ src/spdlog.cpp -Iinclude -std=c++14 -DSPDLOG_COMPILED_LIB
	
Compile

    clang++ -c log_ctest.cpp -Iinclude -std=c++14 -DSPDLOG_COMPILED_LIB
    cc ex.c -c
 
then link

    clang++ ex.o -o ex log_ctest.o spdlog.o 

    cinterop -def spdlog.def -copt -I. -o spdlog
    kotlinc spd_test.kt -o spd_test -l spdlog -linker-options log_ctest.o -linker-options  spdlog.o

Working example: 
    
    cinterop -def src/nativeInterop/cinterop/libgit2.def -copt -I/usr/local/include/ -o libgit2
    kotlinc src/gitChurnMain/kotlin/ -library libgit2 -o gitchurn -e sample.gitchurn.main
    
Do it all:

    clang++ -c spdlog.cpp log_ctest.cpp  -std=c++14 -Iinclude -DSPDLOG_COMPILED_LIB
    cc ex.c -c
    cinterop -def spdlog.def -compiler-options -I. -o spdlog
    kotlinc spd_test.kt -o spd_test -l spdlog -linker-options log_ctest.o -linker-options  spdlog.o

    $ ./spd_test.kexe 
    Hey You
    [2019-07-03 21:03:06.653] [console] [info] This is spdlog console: Using external logger



