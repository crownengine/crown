#include "Crown.h"

using namespace crown;

int main(int argc, char** argv)
{
	Device* engine = device();

	engine->init(argc, argv);

	// Begin Test
	ResourceId rid = engine->resource_manager()->load("sound", "sounds/untrue");
	engine->resource_manager()->flush();

	SoundResource* stream = (SoundResource*)engine->resource_manager()->get(rid);

	OggDecoder decoder((char*)stream->data(), stream->size());

	while(decoder.stream())
	{
		CE_LOGI("size: %d", decoder.size());
	}

	engine->shutdown();

	return 0;
}