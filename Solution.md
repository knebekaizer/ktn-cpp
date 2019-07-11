0. Parse AST
0. Visit recursively, collect supported entries
0. For each entity in list generate wrappers

The point is: use Kotlin/C binding only, while C wrapper impl to be compiled and linked by c++ compiler 

Wrappers are:

extern "C" { ... }
- wrapper funtions for free C++ functions and member functions, e.g.
- fake forward declarations to keep type checks (C++ type tracking):


[[file://x.h]]
[[file://x.cpp]]

kotlin:
- Conveniency classes to wrap ugly C binding into class methods
