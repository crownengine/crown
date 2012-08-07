#include "SignalSlot.h"
#include <iostream>

using namespace std;
using namespace Crown;

class A
{
  public:
    void OnEvent(void* obj, const char* args)
    {
      cout << "Mr.Tambourine said '" << args << "'" << endl;
    }

    A():
      mSlot(this, &A::OnEvent)
    { }

    void fakabuu(Signal<const char*>& signal)
    {
      mSlot.Attach(signal);
    }

  private:
    Slot<A, const char*> mSlot;
};

int main() {

  Signal<const char*> mySignal;
  A a;
  a.fakabuu(mySignal);

  mySignal.Fire(NULL, "She sells sea shells by the sea shore!");

	return 0;
}
