#include "Thread.h"
#include "Log.h"

#include <unistd.h>

using namespace crown;

int32_t first_function(void* ft)
{
	Thread* thread = (Thread*)ft;

	while(thread->is_running())
	{
		Log::i("I'm in the first thread");
	}

	return 0;
}
	
int main()
{
	Thread ft("first-thread");

	ft.start(first_function, &ft);

	sleep(2);

	ft.stop();

	return 0;
}