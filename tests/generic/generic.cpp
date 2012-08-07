#include "Generic.h"
#include <iostream>
#include <stdio.h>
#include "Str.h"

using namespace std;
using namespace Crown;

int main()
{
	Generic g1(5), g2(5.4f);
	Generic g3("[Lol asd]");
	Generic g4("-2147483648");
	Generic g5("4294967295");
	Generic g6("3.1415");

	Str s1, s2, s3;
	g1.asStr(s1);
	g2.asStr(s2);
	g3.asStr(s3);
	int i1 = 0;
	unsigned int ui1 = 0;
	float f1 = 0.0f;
	g4.asInt(i1);
	g5.asUInt(ui1);
	g6.asFloat(f1);

	GenericList list;
	list.Append("xD");
	list.Append(1);
	list.Append(true);
	list.Append(15.4f);

	Generic gList = list;

	Str sList;
	gList.asStr(sList);

	cout << s1.c_str() <<" "<< s2.c_str() <<" "<< s3.c_str() << endl;
	cout << i1 << " " << ui1 << " " << f1 << endl;
	cout << sList.c_str() << endl;
	cout << list.ToStr().c_str() << endl;

  getchar();
}
