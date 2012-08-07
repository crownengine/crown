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

#include "ParticleManager.h"

namespace Crown
{

ParticleManager::ParticleManager()
{

}

ParticleManager::~ParticleManager()
{
	for(int i = 0; i < mEmitters.GetSize(); i++)
	{
		delete mEmitters[i];
	}
}

void ParticleManager::AddEmitter(ParticleEmitter* emitter)
{
	mEmitters.Append(emitter);
	emitter->mParticleManager = this;
}

void ParticleManager::Update(real dt)
{
	int i = 0;
	while (i < mEmitters.GetSize())
	{
		if (mEmitters[i]->mEmitterLifeSeconds <= 0.0 && mEmitters[i]->mEmitterAlpha <= 0.0)
		{
			delete mEmitters[i];
			mEmitters.Remove(i);
		}
		else
		{
			mEmitters[i]->Update(dt);
			i++;
		}
	}
}

void ParticleManager::Render()
{
	for(int i = 0; i < mEmitters.GetSize(); i++)
	{
		mEmitters[i]->Render();
	}
}

} //namespace Crown
