#pragma once

#define VSM_PI 3.14159265359

namespace VSM
{
	template<typename T>
	constexpr T Sqr(const T& x)
	{
		return x * x;
	}

	template<typename T>
	constexpr T Gauss(T x, T sigma)
	{
		return (T)1.0 / ((T)sqrt(2.0 * VSM_PI) * sigma) * exp(-Sqr(x) / ((T)2.0 * Sqr(sigma)));
	}

	template<typename T>
	constexpr T ApproxGaussKernellSize(T sigma)
	{
		return (T)2.0 * (T)VSM_PI * sigma;
	}
}