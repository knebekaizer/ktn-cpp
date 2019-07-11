//
// Created by Vladimir Ivanov on 2019-07-10.
//

#ifdef __cplusplus__
extern "C" {
#endif

typedef struct MyClass_ MyClass_; // match class MyClass
typedef struct std_string_ std_string_; // match class std::string
typedef struct SomeClass_ SomeClass_; // match class Smth

void MyClass_foo_0(MyClass_* self);

// where I got s from?
void MyClass_foo_1(MyClass_* self, int k, std_string_ const* s);

// alternatively, use temporary for const ref binding:
void MyClass_foo_2(MyClass_* self, int k, const char* cstr);

// normal non-const CPointer (stored)
void MyClass_foo_3(MyClass_* self, int k, std_string_* s);

// using temporary:
void MyClass_foo_4(MyClass_* self, int k, const char* s, SomeClass_ const* x);

// const this (receiver)
int MyClass_bar(MyClass_ const* self);

// the same for virtual
void MyClass_v_foo(MyClass_* self);

// statioc:
void MyClass_s_foo(SomeClass_* smth);

#ifdef __cplusplus__
}
#endif