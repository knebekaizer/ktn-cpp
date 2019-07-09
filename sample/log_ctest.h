class MyClass {
	int x;
	void foo();
};
MyClass mc;

struct Logger;
typedef struct Logger Logger;

#ifdef __cplusplus
extern "C" {
#endif
	
	Logger* log_create(void);
	void log_write(Logger* log, const char* msg);
	
	const char* cfun(const char*);
	const signed char* scfun(const signed char*);
	const unsigned char* ucfun(const unsigned char*);

#ifdef __cplusplus
} // __cplusplus
#endif