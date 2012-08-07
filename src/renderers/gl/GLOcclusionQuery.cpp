/*
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

#include <GL/glew.h>
#include "GLOcclusionQuery.h"

namespace Crown
{

GLOcclusionQuery::GLOcclusionQuery() :
	mQueryObject(0)
{
	glGenQueries(1, &mQueryObject);
}

GLOcclusionQuery::~GLOcclusionQuery()
{
	glDeleteQueries(1, &mQueryObject);
}

void GLOcclusionQuery::BeginQuery()
{
	glBeginQuery(GL_SAMPLES_PASSED, mQueryObject);
}

void GLOcclusionQuery::EndQuery()
{
	glEndQuery(GL_SAMPLES_PASSED);
}

uint GLOcclusionQuery::GetQueryResult()
{
	uint passedCount;

	glGetQueryObjectuiv(mQueryObject, GL_QUERY_RESULT, &passedCount);

	return passedCount;
}

bool GLOcclusionQuery::IsResultAvailable()
{
	uint available;

	glGetQueryObjectuiv(mQueryObject, GL_QUERY_RESULT_AVAILABLE, &available);

	return available != 0;
}

} // namespace Crown

