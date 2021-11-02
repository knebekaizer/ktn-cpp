namespace std {
template <typename T> class Foo {
	void foo(const double f) const;
	template <class U> void foo(int i = 101) { ++i; };
};
template <typename T> void Foo<T>::foo(const double f) const { auto g = f * 2; }

class Bar {};
Foo<Bar> instFoo;
}
