#include "Crown.h"

using namespace Crown;

int main(int argc, char** argv)
{
	XMLReader xml;
	xml.LoadFile("res/window.xml");
	xml.Print();

	getchar();

	return 0;
}

