#include "Dictionary.h"
#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
  Dictionary<int, int> dict;
  int k = 20;

  for(int i=0; i<k; i++)
    dict[i] = 10*i;

  Dictionary<int, int>::Enumerator e = dict.getBegin();
  while (e.next())
  {
    cout << "[" << e.current().key << "," << e.current().value << "] ";
  }
  cout << endl;

  Dictionary<int, int>::Enumerator eb = dict.getEnd();
  while (eb.prev())
  {
    cout << "[" << eb.current().key << "," << eb.current().value << "] ";
  }
  cout << endl;

  getchar();
}
