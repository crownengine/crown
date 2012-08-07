#include <iostream>
#include <stdio.h>
#include "Timer.h"

//#define RBTREE_VERIFY
#include "Dictionary.h"
#include "Str.h"

#include <map>

using namespace std;
using namespace Crown;

void timeCompareWithStlMap();
void dictRemove();

int main()
{
	timeCompareWithStlMap();
	dictRemove();
	
  getchar();
}

void dictRemove()
{
	Dictionary<int, Crown::Str> dict;
	
	for(int i=0; i<20; i++)
	{
		dict[i] = "item " + Str(i*5);
	}
	
	dict.Remove(7);
	
	cout << "Dictionary with item 10 removed:" << endl;
	
	Dictionary<int, Crown::Str>::Enumerator e = dict.getBegin();
	while (e.next())
	{
		cout << "dict[" << e.current().key << "] = " << e.current().value.c_str() << endl;
	}
	
	dict.Clear();
}

void timeCompareWithStlMap()
{
  Timer* tim = Timer::GetInstance();
  tim->Reset();
  int k = 1000000;
  int dictAddTime, dictRemoveTime, mapAddTime, mapRemoveTime;
  int dictFindTime, mapFindTime;

  map<int, int> stlmap;
  Dictionary<int, int> dict;

  //------Dict performance test------
  tim->Reset();
  for(int i=0; i<k; i++)
  {
    dict[i] = i;
    //dict.add(i, i);
  }
  dictAddTime = tim->GetMicroseconds();
  cout << "Elements added to Dictionary: " << dict.GetSize() << endl;

  tim->Reset();
  for(int i=0; i<k; i++)
  {
    assert(dict[i] == i);
  }
  dictFindTime = tim->GetMicroseconds();
  cout << "Elements searched" << endl;

  tim->Reset();
  for(int i=0; i<k; i++)
  {
    dict.Remove(i);
  }
  dictRemoveTime = tim->GetMicroseconds();
  cout << "Elements after removal on Dictionary: " << dict.GetSize() << endl;

  //------Map performance test------
  tim->Reset();
  for(int i=0; i<k; i++)
  {
    stlmap[i] = i;
    //stlmap.insert(map<int, int>::value_type(i, i));
  }
  mapAddTime = tim->GetMicroseconds();
  cout << "Elements added to StlMap: " << stlmap.size() << endl;

  tim->Reset();
  for(int i=0; i<k; i++)
  {
    assert(stlmap[i] == i);
  }
  mapFindTime = tim->GetMicroseconds();
  cout << "Elements searched" << endl;

  tim->Reset();
  for(int i=0; i<k; i++)
  {
    stlmap.erase(i);
  }
  mapRemoveTime = tim->GetMicroseconds();
  cout << "Elements after removal on StlMap: " << stlmap.size() << endl;

  cout << "Dictionary:" << endl;
  cout << "Add: " << dictAddTime << " us, Remove: " << dictRemoveTime << " us, Find: " << dictFindTime << " us" << endl;
  cout << "Add: " << 1.0f*dictAddTime/k << " us/add, Remove: " << 1.0*dictRemoveTime/k << " us/remove, Find: " << 1.0*dictFindTime/k << " us/find" << endl;

  cout << "StlMap:" << endl;
  cout << "Add: " << mapAddTime << " us, Remove: " << mapRemoveTime << " us, Find: " << mapFindTime << " us" << endl;
  cout << "Add: " << 1.0f*mapAddTime/k << " us/add, Remove: " << 1.0*mapRemoveTime/k << " us/remove, Find: " << 1.0*mapFindTime/k << " us/find" << endl;		
}
