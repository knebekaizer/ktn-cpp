namespace std {

void dummy();
auto empty = []{};

class Bar {};
class Baz;
template <int I> class BarTmpl;
template <typename T> struct Foo {
	void foo(const double f) const;
	template <class Bar, int N, template <int> class BarTmpl, void (F)() = dummy> constexpr long fooTmpl(int i = 101) {  return ++i; };
};

template <typename T> void Foo<T>::foo(const double f) const { auto g = f * 2; }

Foo<Bar> instFoo;

Foo<int> fint;
constexpr auto z = fint.fooTmpl<Foo<Bar>, 42, BarTmpl>();
}
