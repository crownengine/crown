#include "LuaCompiler.h"
#include "FileStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
LuaCompiler::LuaCompiler(const char* root_path, const char* dest_path, const char* resource, uint32_t seed) :
	Compiler(root_path, dest_path, resource, seed),
	m_file_size(0),
	m_file_data(NULL)
{

}

//-----------------------------------------------------------------------------
bool LuaCompiler::compile()
{
    Filesystem fs(root_path());

    char tmp_resource[os::MAX_PATH_LENGTH];

    string::strcpy(tmp_resource, resource_path());
    string::strcat(tmp_resource, ".script");

    if (!fs.exists(tmp_resource))
    {
        os::printf("Resource cannot be found.\n");
        return false;
    }

    FileStream* file = (FileStream*)fs.open(tmp_resource, SOM_READ);

    m_file_size = file->size();

    if (m_file_size == 0)
    {
        return false;
    }

    m_file_data = new char[m_file_size];
    
    // Copy the entire file into the buffer
    file->read(m_file_data, m_file_size);

    // Prepare for writing
    Compiler::prepare_header(m_file_size);

    return true;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write()
{
    Compiler::write_header();

    FileStream* file = Compiler::destination_file();

    file->write(&m_file_size, sizeof(uint32_t));
    file->write(m_file_data, m_file_size);
}

} // namespace crown
