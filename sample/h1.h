//
// Created by Vladimir Ivanov on 2019-07-15.
//

#ifndef KTN_CPP_H1_H
#define KTN_CPP_H1_H

typedef char A16[16];

typedef A16* A16Ptr;
typedef A16Ptr& A16PtrRef;

A16 a16;
A16Ptr a16Ptr;
A16PtrRef a16PtrRef = a16Ptr;
//char (*&) wtf[16];

struct S;
typedef struct S S;

S* pStr;

namespace ns {
class MyClass {
public:
	void foo(int x);
	char* getConst() const;
	static MyClass* create();
};

MyClass* myClassPtr;
}

using MyClassT = ns::MyClass;
ns::MyClass const* freeFoo(ns::MyClass const&);
ns::MyClass const* freeFoo(const ns::MyClass byValue);
ns::MyClass const& freeFoo(MyClassT* byPtr);

using MyClassRef = MyClassT&;
typedef MyClassRef MyClassT2;
int freeFoo(MyClassT2 byRefTypedef);

auto autoFun(long& x) {
	return &x;
}

#endif //KTN_CPP_H1_H
