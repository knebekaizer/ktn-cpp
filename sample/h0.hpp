
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

TheStruct bar(TheStruct* s);

TheStruct* create();

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
