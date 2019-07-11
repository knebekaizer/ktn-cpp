struct TheStruct{
};
struct TheStruct theStruct;
class TheClass {
};
TheClass theClass;

class MyClass{
public:
	int x;
	const char v[4] = "abc";
	void foo();
	void foo(int k, const std::string& s); // overload
	void foo(int k, std::string& s); // overload, non-const reference
	void foo(int k, std::string s, Smth const& x); // overload, string by value

	int bar() const;
	virtual void v_foo();
	static void s_foo(Other Class* smth);
};

void MyClass::s_foo(MyClass* self) {
	return self->v_foo();
}

#include <string>
#include <vector>

namespace foo {
    void bar(std::string s);
    class Bar {
    public:
	    Bar() = delete;
	    Bar(const Bar&);
	    Bar(Bar&);
	    Bar(int x, int y);
        std::vector<int> vec;
        class InnerBar {
        } inner;
    };
}

struct Logger;
typedef struct Logger Logger;

#ifdef __cplusplus
extern "C" {
#endif
	
	Logger* log_create(void);
	void log_write(Logger* log, const char* msg);

#ifdef __cplusplus
} // __cplusplus
#endif