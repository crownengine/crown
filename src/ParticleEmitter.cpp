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

#include "ParticleEmitter.h"
#include "Device.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "Random.h"
#include "Timer.h"
#include <GL/glew.h> // FIXME

namespace crown
{

Random ParticleEmitterRandom(Timer().GetMicroseconds());

ParticleEmitter::ParticleEmitter(Vec3 source, real intervalSeconds, real emitterLifeSeconds, real angle, real angleSpan):
	mSourcePosition(source), mIntervalSeconds(intervalSeconds), mIntervalCounter(0.0), 
	mSpawnAngle(angle), mSpawnAngleSpan(angleSpan),
	mEmitterLifeSeconds(emitterLifeSeconds), 
	mIsEmitterFading(false), mEmitterAlpha(1.0)
{
	mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();

	mTex = GetTextureManager()->Load("res/particle.bmp");
}

ParticleEmitter::~ParticleEmitter()
{
	for(int i = 0; i < mParticles.GetSize(); i++)
	{
		delete mParticles[i];
	}
	//delete mVertexBuffer;
}

void ParticleEmitter::Update(real dt)
{
	if (mEmitterLifeSeconds >= dt)
	{
		mIntervalCounter += dt;
		while (mIntervalCounter >= mIntervalSeconds)
		{
			mIntervalCounter -= mIntervalSeconds;

			Particle* p = new Particle();

			p->Position = mSourcePosition;
			p->LifeSeconds = 1.5;
			real angle = (ParticleEmitterRandom.GetUnitFloat() * 2 - 1) * mSpawnAngleSpan + mSpawnAngle;
			real len = ParticleEmitterRandom.GetUnitFloat() * 3.5 + 0.5;
			p->Velocity = Vec3(Math::Cos(angle)*len, Math::Sin(angle)*len, 0.0);
			p->Friction = p->Velocity * (-0.1);

			mParticles.Append(p);
		}
		mEmitterLifeSeconds -= dt;
	}
	else
	{
		mEmitterLifeSeconds = 0.0;
		if (mEmitterAlpha > 0.0)
		{
			mEmitterAlpha -= 0.05;
		}
		else
		{
			mEmitterAlpha = 0.0;
		}
	}

	UpdateParticles(dt);
}

void ParticleEmitter::UpdateParticles(real dt)
{
	int i = 0;
	while (i < mParticles.GetSize())
	{
		Particle* p = mParticles[i];
		if (p->LifeSeconds <= 0)
		{
			mParticles.Remove(i);
			continue;
		}

		p->LifeSeconds -= dt;

		if (p->Velocity.GetSquaredLength() < p->Friction.GetSquaredLength())
		{
			p->Velocity = Vec3(0.0, 0.0, 0.0);
		}
		else
		{
			p->Velocity += p->Friction;
			p->Position += p->Velocity;
		}

		i++;
	}
}

void ParticleEmitter::Render()
{
	Renderer* renderer = GetDevice()->GetRenderer();
			
	renderer->SetTexture(0, mTex);

	renderer->_SetBlending(true);
	renderer->_SetBlendingParams(BE_FUNC_ADD, BF_SRC_ALPHA, BF_ONE, Color4::WHITE);
	//glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	renderer->_SetPointSize(8);
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE_ARB, GL_TRUE );
	renderer->_SetPointSprite(true);

	glBegin(GL_POINTS);
	glTexCoord2f(0.5, 0.5);
	for(int i = 0; i < mParticles.GetSize(); i++)
	{
		Particle* p = mParticles[i];
		glColor4f(1.0, 0.0, 0.0, mEmitterAlpha);
		glVertex2f(p->Position.x, p->Position.y);
		//data[dataIdx++] = p->Position.x;
		//data[dataIdx++] = p->Position.y;
		//data[dataIdx++] = p->Position.z;
		//data[dataIdx++] = p->UV.x;
		//data[dataIdx++] = p->UV.y;
	}
	glEnd();

	renderer->_SetPointSprite(false);

	renderer->_SetBlendingParams(BE_FUNC_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA, Color4::WHITE);

	////6 reals for each particle, 3*position + 2*uv
	////real* data = new real[mParticles.GetSize() * 5];
	////int dataIdx = 0;
	//GetDevice()->GetRenderer()->_SetPointSprite(true);
	//glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	//real _maxSize;
	//glGetFloatv(GL_POINT_SIZE_MAX_ARB, &_maxSize );


	//glColor3f(1.0, 0.0, 0.0);
	//glPointSize(_maxSize);

	////mVertexBuffer->SetVertexData(VBM_TEXTURE_COORDS, data, mParticles.GetSize());
	////delete[] data;

	////GetDevice()->GetRenderer()->RenderPointBuffer(mVertexBuffer);
}

} //namespace crown
