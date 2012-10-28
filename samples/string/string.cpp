#include <iostream>
#include "../core/containers/Str.h"

using namespace Crown;
using namespace std;

template<typename T>
void TestResult(T actual, T expected);
void SplitTest();
void RemoveTest();
void ReplaceTest();

int tests = 0;
int successes = 0;

int main() {

	SplitTest();

	cout << endl;

	RemoveTest();

	cout << endl;

	ReplaceTest();

	cout << endl;
	cout << "Test result: " << successes << "/" << tests << endl;
	cout << endl;

	return 0;
}

void SplitTest()
{
	Str str = " abc def ghi,e asdgfe ", rebuilt;
	List<Str> split;

	str.Split(' ', split);

	for(int i = 0; i < split.GetSize(); i++)
	{
		rebuilt += split[i];
		if (i < split.GetSize()-1)
			rebuilt += " ";
	}

	cout << " = Split test = " << endl;
	cout << "- Split count ";
	TestResult<int>(split.GetSize(), 6);

	cout << "- Rebuilding split string ";
	TestResult<const Str&>(rebuilt, str);
}

void RemoveTest()
{
	Str str = "il gatto salta di notte";
	Str strBegin = str;
	Str strEnd = str;
	Str strMiddle = str;

	strBegin.Remove(0, 3);
	strEnd.Remove(strEnd.GetLength()-6, strEnd.GetLength());
	strMiddle.Remove(3, 9);

	cout << " = Remove test = " << endl;

	cout << "- Remove from begin ";
	TestResult<const Str&>(strBegin, "gatto salta di notte");
	cout << str.c_str() << " -> " << strBegin.c_str() << endl << endl;

	cout << "- Remove from end ";
	TestResult<const Str&>(strEnd, "il gatto salta di");
	cout << str.c_str() << " -> " << strEnd.c_str() << endl << endl;

	cout << "- Remove from middle ";
	TestResult<const Str&>(strMiddle, "il salta di notte");
	cout << str.c_str() << " -> " << strMiddle.c_str() << endl << endl;
}

void ReplaceTest()
{
	Str str = "il gatto salta di notte";
	Str strBegin = str;
	Str strEnd = str;
	Str strMiddle = str;

	strBegin.Replace("il gatto", "la gatta non");
	strEnd.Replace("notte", "largo anticipo");
	strMiddle.Replace("salta", "dorme");

	cout << " = Replace test = " << endl;

	cout << "- Replace from begin ";
	TestResult<const Str&>(strBegin, "la gatta non salta di notte");
	cout << str.c_str() << " -> " << strBegin.c_str() << endl << endl;

	cout << "- Replace from end ";
	TestResult<const Str&>(strEnd, "il gatto salta di largo anticipo");
	cout << str.c_str() << " -> " << strEnd.c_str() << endl << endl;

	cout << "- Replace from middle ";
	TestResult<const Str&>(strMiddle, "il gatto dorme di notte");
	cout << str.c_str() << " -> " << strMiddle.c_str() << endl << endl;
}

template<typename T>
void TestResult(T actual, T expected)
{
	tests++;
	if (actual == expected)
	{
		cout << "[Ok]" << endl;
		successes++;
	}
	else
		cout << "[Failure]" << endl;
}
