#include "List.h"
#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
  List<int> list;
  int k = 20;

  for(int i=0; i<k; i++)
    list.Append(i);

  EnumeratorHelper<int> e = list.getBegin();
  while (e.next())
  {
    cout << e.current() << " ";
  }
  cout << endl;

  EnumeratorHelper<int> eb = list.getEnd();
  while (eb.prev())
  {
    cout << eb.current() << " ";
  }
  cout << endl;

  getchar();
}
