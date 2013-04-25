#pragma once

#include "Compiler.h"
#include "FileStream.h"

namespace crown
{

class LuaCompiler : public Compiler
{

public:
					LuaCompiler(const char* root_path, const char* dest_path, const char* resource, uint32_t seed);

	bool			compile();
	void 			write();

private:

	uint32_t		m_file_size;
	char*			m_file_data;
};

} // namespace crown