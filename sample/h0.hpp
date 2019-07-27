
//namespace ns {

class TheStruct
{
public:
	int iPub = 42;

	virtual int foo(const TheStruct*);
//	int foo(const TheStruct*);
private:
//	TheStruct* fct() const;

private:
	int iPriv;
};

TheStruct bar(TheStruct* s);
TheStruct* create();

//}