#pragma once

#include <cmath>

// TPT State Variable Filter - per-sample processing.
struct TPTSVF
{
	float s1 = 0.f; // Integrator 1 state (BP)
	float s2 = 0.f; // Integrator 2 state (LP)
	float g = 0.f;  // tan(pi * fc / fs)
	float R = 1.f;  // Damping: R = 1 / (2 * Q)
	float D = 1.f;  // Precomputed denominator

	float lp = 0.f; // Low-pass output
	float bp = 0.f; // Band-pass output
	float hp = 0.f; // High-pass output

	// Fast tanh (Pade approximant, ~0.1% error)
	float fastTanh(float x) 
	{
		float x2 = x * x;
		return x * (27.0f + x2) / (27.0f + 9.0f*x2);\
	}

	// Even faster: cubic soft clip
	float softClip(float x) 
	{
		if (x > 1.0f) return 1.0f;
		if (x < -1.0f) return -1.0f;
		return x - xxx / 3.0f;
	}

	void setParams(float cutoffHz, float Q, float fs)
	{
		constexpr float pi = 3.14159265358979323846f;
		g = std::tan(pi * cutoffHz / fs);
		R = 1.f / (2.f * Q);
		D = 1.f / (1.f + 2.f * R * g + g * g);
	}

	float process(float x)
	{
		// Solve the algebraic loop.
		hp = (x - (2.f * R + g) * s1 - s2) * D;

		// Compute outputs from integrators.
		const float v1 = g * hp;
		bp = v1 + s1;
		const float v2 = g * bp;
		lp = v2 + s2;

		// Update states (equivalent to s = 2 * output - previous state).
		s1 = bp + v1;
		s2 = lp + v2;

		return lp;
	}

	// Optional saturated path; the module continues to use process().
	float processLinearWithSaturation(float x)
	{
		// Linear ZDF core. Compute both integrator increments before saturation.
		hp = (x - (2.f * R + g) * s1 - s2) * D;
		const float v1 = g * hp;
		bp = v1 + s1;
		const float v2 = g * bp;
		lp = v2 + s2;

		// Saturate outputs before updating states.
		bp = std::tanh(bp);
		lp = std::tanh(lp);
		s1 = bp + v1;
		s2 = lp + v2;

		return lp;
	}
};
