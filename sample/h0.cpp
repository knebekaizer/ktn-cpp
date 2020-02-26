#pragma GCC diagnostic ignored "-Wunused-function"
#include "h0.hpp"

#include <iostream>


using namespace std;

namespace ns {

void NoName::noNameMember() {
	cout << __PRETTY_FUNCTION__ << " invoked" << endl;
}

int TheStruct::s_fun() {
	static int counter = 777;
	cout << __PRETTY_FUNCTION__ << " invoked" << endl;
	return counter++;
}

int TheStruct::foo(const TheStruct* x) {
	int res = x == this;
	cout << "This is TheStruct::foo: result is iPub + (int)(param == this): " << iPub + res << endl;
	return iPub + res;
}

TheStruct::TheStruct() : iPub(99) {}

TheStruct::TheStruct(int i) : iPub(i) {}

/*
TheStruct::~TheStruct() {
	cout << "~TheStruct dtor called" << endl;
}
*/

TheStruct bar(TheStruct* s) {
	if (s)
		return *s;
	else
		return * new TheStruct();
}

TheStruct* create() {
	return new TheStruct();
}

} // ns

TheStruct* create() {
	cout << __PRETTY_FUNCTION__ << " declared in global ns" << endl;
	return nullptr;
}

::TheStruct* ns2::create() {
	cout << __PRETTY_FUNCTION__ << " declared in ns2" << endl;
	return nullptr;
}

#ifdef MAIN

#include "trace.h"
int main() {
	using namespace lamb;
	log_info << "A stateful lambda is here:";
	TraceX(lamb_42);
	TraceX(lamb_2_42);
	TraceX(getLamb()(3));
}
#endif // MAIN