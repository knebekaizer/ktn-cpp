
namespace ns {

struct TheStruct
{
public:
	int iPub;

	void foo();

private:
	int iPriv;
};

TheStruct theStruct;

int bar(TheStruct& s);

}
