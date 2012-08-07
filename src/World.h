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

#pragma once

namespace Crown
{

/**
	The game world.

	World represents the place where interactions between all the
	game entities occur. It holds the systems which 
*/
class World
{

public:

				World();
				~World();

	void		SetTimeOfDay(ushort seconds);		//!< Sets the time of day in seconds
	ushort		GetTimeOfDay() const;				//!< Returns the time of day in seconds

	void		Update(uint dt);

private:

	ushort		mTimeOfDay;			// Time of day in seconds. (60 * 60 * 24 secs)
	ushort		mTimeOfDaySpeed;	// Speed of seconds in 1/100 of seconds. (100 = 1 game second equals 1 real second)

	float		mGravity;			// Gravity of the world in meters/seconds

	Camera		mCamera;			// Camera (i.e. the point of view)
	Skybox		mSkybox;			// The skybox
};

} // namespace Crown

