/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/math.h>
#include <bx/timer.h>
#include <bx/file.h>

#include <math.h>

typedef float (*MathFn)(float);

template<MathFn mfn>
float mathTest(const char* _name)
{
	bx::WriterI* writer = bx::getStdOut();
	int64_t elapsed = -bx::getHPCounter();

	float result = 0.0f;
	const float max = 1389.0f;

	for (float xx = 0.0f; xx < max; xx += 0.1f)
	{
		result += mfn(xx);
	}

	elapsed += bx::getHPCounter();
	bx::writePrintf(writer, "%-20s: %15f\n", _name, double(elapsed) );

	return result;
}

float rsqrt(float _a)
{
	return 1.0f/::sqrtf(_a);
}

void math_bench()
{
	bx::WriterI* writer = bx::getStdOut();
	bx::writePrintf(writer, "Math bench\n\n");

	mathTest<  ::sqrtf    >("  ::sqrtf");
	mathTest<bx::sqrtRef  >("bx::sqrtRef");
	mathTest<bx::sqrtSimd >("bx::sqrtSimd");
	mathTest<bx::sqrt     >("bx::sqrt");

	bx::writePrintf(writer, "\n");
	mathTest<  ::rsqrt    >("  ::rsqrtf");
	mathTest<bx::rsqrtRef >("bx::rsqrtRef");
	mathTest<bx::rsqrtSimd>("bx::rsqrtSimd");
	mathTest<bx::rsqrt    >("bx::rsqrt");

	bx::writePrintf(writer, "\n");
	mathTest<  ::sinf >("  ::sinf");
	mathTest<bx::sin  >("bx::sin");

	bx::writePrintf(writer, "\n");
	mathTest<  ::asinf>("  ::asinf");
	mathTest<bx::asin >("bx::asin");

	bx::writePrintf(writer, "\n");
	mathTest<  ::cosf >("  ::cosf");
	mathTest<bx::cos  >("bx::cos");

	bx::writePrintf(writer, "\n");
	mathTest<  ::acosf>("  ::acosf");
	mathTest<bx::acos >("bx::acos");

	bx::writePrintf(writer, "\n");
	mathTest<  ::tanf >("  ::tanf");
	mathTest<bx::tan  >("bx::tan");

	bx::writePrintf(writer, "\n");
	mathTest<  ::atanf>("  ::atanf");
	mathTest<bx::atan >("bx::atan");
}
