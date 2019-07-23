
//namespace ns {

class TheStruct
{
public:
	int iPub;

	void foo(TheStruct*);
	void foo() const;

private:
	int iPriv;
};

TheStruct bar(TheStruct* s);

//}