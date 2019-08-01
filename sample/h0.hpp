
namespace ns {

typedef class {
public:
	void noNameMember();
} NoName;

class TheStruct {
public:
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

}

inline template <typename T> int funcTmpl() {
	return 42;
}

template <typename T> int funcTmpl2();
