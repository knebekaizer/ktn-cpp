//
// Created by Vladimir Ivanov on 2019-07-10.
//

// whatever you need:
#include <string>

class SomeClass;

class MyClass{
public:
	int x;
	void foo();
	void foo(int k, const std::string& s); // overload
	void foo(int k, std::string& s); // overload, non-const reference
	void foo(int k, std::string s, SomeClass const& x); // overload, string by value

	int bar() const;
	virtual void v_foo();
	static void s_foo(SomeClass& smth);
};

// wrappers implementation
#include "x.h"


void MyClass_foo_0(MyClass_* self) { return ((MyClass*)self)->foo(); }

// where I got s from?
void MyClass_foo_1(MyClass_* self, int k, std_string_ const* s) { return ((MyClass*)self)->foo(k, *(std::string const*)s); }

// alternatively, use temporary for const ref binding:
void MyClass_foo_2(MyClass_* self, int k, const char* cstr) { return ((MyClass*)self)->foo(k, cstr); }

// normal non-const CPointer (stored)
void MyClass_foo_3(MyClass_* self, int k, std_string_* s) { return ((MyClass*)self)->foo(k, *(std::string const*)s); }

// using temporary:
void MyClass_foo_4(MyClass_* self, int k, const char* s, SomeClass_ const* x) { return ((MyClass*)self)->foo(k, s, *(SomeClass const*)s); }

// const this (receiver)
int MyClass_bar(MyClass_ const* self) { return ((MyClass const*)self)->bar(); }

// the same for virtual
void MyClass_v_foo(MyClass_* self) { return ((MyClass*)self)->v_foo(); }

// statioc:
void MyClass_s_foo(SomeClass_* smth) { return MyClass::s_foo(*(SomeClass*)smth); }

