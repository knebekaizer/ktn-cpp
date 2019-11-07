//#include "inttypes.h"
//#include "simd/simd.h"

//#include "Accelerate/Accelerate.h"

typedef float                   vFloat          __attribute__ ((__vector_size__ (16)));
vFloat vF16;
__attribute__ ((__vector_size__ (8))) float vF8;

typedef __attribute__((__ext_vector_type__(4))) float simd_float4;
simd_float4 sf4;

__int128_t i128;

namespace ns {

typedef class {
public:
	void noNameMember();
} NoName;


class TheStruct {
public:
	struct Inner {
		struct Inner2 {
			int x;
		} inner2;
	};

	static int s_fun();

	TheStruct();

	TheStruct(int i);

	~TheStruct() = default;


	int iPub = 42;

	virtual int foo(const TheStruct*);
//	int foo(const TheStruct*);

	template <class X> void fooTmplMember() const;
private:
//	TheStruct* fct() const;

private:
	int iPriv;
};

} // ns


namespace {
ns::TheStruct* fooInAnonNamespace();

}

class TheStruct;
TheStruct* create();

namespace ns2 {
	::TheStruct* create();

template <typename T> struct TmplStruct {
public:
	void baz() const {}
};

template <typename T> class TmplClass {
public:
	void baz() const {}
};

template <class T> void funTmpl();

} // ns2


namespace ns {
	TheStruct bar(TheStruct* s);
	TheStruct* create();
	struct NSAgain {
		int nsAgainMember;
	};
}
