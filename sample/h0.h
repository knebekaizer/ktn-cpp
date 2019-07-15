
namespace ns {

struct TheStruct
{
public:
	int iPub;

	void foo();

private:
	int iPriv;
};

int bar(TheStruct& s);

}

ns::TheStruct theStruct;
