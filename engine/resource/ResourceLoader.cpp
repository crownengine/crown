/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ResourceLoader.h"
#include "ResourceRegistry.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceLoader::ResourceLoader(Bundle& bundle, Allocator& resource_heap) :
	Thread("resource-loader"),
	m_bundle(bundle),
	m_resource_heap(resource_heap),
	m_num_requests(0),
	m_requests(default_allocator())
{
	for (uint32_t i = 0; i < MAX_LOAD_REQUESTS; i++)
	{
		m_results[i].status = LRS_NO_INFORMATION;
		m_results[i].data = NULL;
	}
}

//-----------------------------------------------------------------------------
LoadResourceId ResourceLoader::load_resource(uint32_t type, ResourceId resource)
{

	m_requests_mutex.lock();

	LoadResourceId lr_id = m_num_requests++;
	LoadResource lr;
	lr.id = lr_id;
	lr.type = type;
	lr.resource = resource;

	m_requests.push_back(lr);

	m_results[lr_id % MAX_LOAD_REQUESTS].status = LRS_QUEUED;

	m_requests_mutex.unlock();

	m_full.signal();

	return lr_id;
}

//-----------------------------------------------------------------------------
LoadResourceStatus ResourceLoader::load_resource_status(LoadResourceId id) const
{
	if (m_num_requests - id > MAX_LOAD_REQUESTS)
	{
		return LRS_NO_INFORMATION;
	}

	return m_results[id % MAX_LOAD_REQUESTS].status;
}

//-----------------------------------------------------------------------------
void* ResourceLoader::load_resource_data(LoadResourceId id) const
{
	if (m_num_requests - id > MAX_LOAD_REQUESTS)
	{
		return NULL;
	}

	return m_results[id % MAX_LOAD_REQUESTS].data;
}

//-----------------------------------------------------------------------------
int32_t ResourceLoader::run()
{
	while (!is_terminating())
	{
		m_requests_mutex.lock();
		while (m_requests.empty())
		{
			m_full.wait(m_requests_mutex);
		}

		LoadResource request = m_requests.front();
		m_requests.pop_front();

		m_requests_mutex.unlock();

		m_results[request.id % MAX_LOAD_REQUESTS].status = LRS_LOADING;

		void* data = resource_on_load(request.type, m_resource_heap, m_bundle, request.resource);

		m_results[request.id % MAX_LOAD_REQUESTS].data = data;
		m_results[request.id % MAX_LOAD_REQUESTS].status = LRS_LOADED;
	}

	return 0;
}

} // namespace crown
