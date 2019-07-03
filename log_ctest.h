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