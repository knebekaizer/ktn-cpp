struct TheStruct{
};
struct TheStruct theStruct;
class TheClass {
};
TheClass theClass;

class MyClass{
public:
	int x;
	void foo();
	virtual void v_foo();
	static void s_foo(MyClass* self);
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
        std::vector<int> vec;
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