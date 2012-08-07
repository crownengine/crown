#include "Crown.h"
#include "OS.h"
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

using namespace Crown;

class ResourceLoader
{
public:

					ResourceLoader() : mDone(false), mProcessed(0)
					{
					}

					~ResourceLoader()
					{
					}

	void			Load(char* name)
					{
						mResourceQueue.Append(name);
						mProcessed = 0;
					}


	void			Flush()
					{
						pthread_create(&thread_id, NULL, &(ResourceLoader::ThreadStarter), this);
					}

	void			LoadAll()
					{
						for (int i = 0; i < mResourceQueue.GetSize(); i++)
						{
							GetFilesystem()->Close(GetFilesystem()->OpenStream(mResourceQueue[i], SOM_READ));
							mProcessed++;
							sleep(1);
						}

						mResourceQueue.Clear();

						mDone = true;
					}

	bool			IsDone() const { return mDone; }

	int				GetProcessed() const { return mProcessed; }
	int				GetPending() const { return mResourceQueue.GetSize(); }

private:

	volatile bool	mDone;
	volatile int	mProcessed;
	pthread_t		thread_id;

	List<char*>		mResourceQueue;

	static void*	ThreadStarter(void* instance)
					{
						reinterpret_cast<ResourceLoader*>(instance)->LoadAll();
					}
};

int main()
{
	Device* device = GetDevice();

	Filesystem* fs = GetFilesystem();

	fs->Init(Str::EMPTY, Str::EMPTY);

	fs->PrintReport();

	char path[256];

//	do
//	{
//		std::cin >> path;

//		FilesystemEntry info;
//		if (fs->GetInfo(fs->GetRootPath(), path, info))
//		{
//			info.PrintReport();
//		}
//	}
//	while (strcmp(path, "=exit") != 0);

//	do
//	{
//		std::cin >> path;

//		const char* extension = Filesystem::GetExtension(path);

//		std::cout << "extension: ";
//		printf("%s\n", extension);
//	}
//	while (strcmp(path, "=exit") != 0);

	fs->CreateFile("ciao.txt2");
	fs->CreateDir("pippo");

	ResourceLoader loader;

	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");
	loader.Load("res/red_north.tga");
	loader.Load("res/red_up.tga");
	loader.Load("res/red_down.tga");
	loader.Load("res/red_west.tga");
	loader.Load("res/red_east.tga");
	loader.Load("res/red_south.tga");



	int pending = loader.GetPending();

	printf("%d\n", pending);

	getchar();

	loader.Flush();

	while (pending != loader.GetProcessed())
	{
		printf("%d/%d\n", loader.GetProcessed(), pending);

		sleep(2);
	}

	return 0;
}

