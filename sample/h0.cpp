#include "h0.hpp"

/*

class TheStruct
{
public:
	int iPub;

	int foo(const TheStruct*);
	TheStruct* fct() const;

private:
	int iPriv;
};

TheStruct bar(TheStruct* s);
 */

#include <iostream>

using namespace std;

extern "C" int32_t test_kniBridge_0 (void* p0, void* p1) {
	return (int32_t)((TheStruct*)p0)->foo((TheStruct*)p1);
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