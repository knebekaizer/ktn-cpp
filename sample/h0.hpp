
//namespace ns {

typedef struct
{
public:
	int iPub;

	void foo();

	struct Inner {
	    int innerInt;
	} inner;

private:
	int iPriv;
} TheStruct;

int bar(TheStruct* s);

//}
/*
ns::TheStruct theStruct;

extern "C" {

ns::TheStruct& xRef = theStruct;
}
*/