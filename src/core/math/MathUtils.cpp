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

#include "MathUtils.h"
#include "Types.h"

namespace Crown
{

const real		Math::PI						= (real)3.1415926535897932;
const real		Math::TWO_PI					= PI * (real)2.0;
const real		Math::HALF_PI					= PI * (real)0.5;
const real		Math::ONEFOURTH_PI				= PI * (real)0.25;

const real		Math::DEG_TO_RAD				= PI / (real)180.0;
const real		Math::RAD_TO_DEG				= (real)1.0 / DEG_TO_RAD;

const real		Math::FOUR_OVER_THREE			= (real)(4.0 / 3.0);
const real		Math::FOUR_OVER_THREE_TIMES_PI	= FOUR_OVER_THREE * PI;

const real		Math::ONE_OVER_THREE			= (real)(1.0 / 3.0);
const real		Math::ONE_OVER_255				= (real)(1.0 / 255.0);

const float		Math::FLOAT_PRECISION			= (real)1.0e-7f;
const double	Math::DOUBLE_PRECISION			= (real)1.0e-9;

} // namespace Crown

