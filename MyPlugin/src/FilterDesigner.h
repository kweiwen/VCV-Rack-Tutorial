#ifndef FilterDesigner_h
#define FilterDesigner_h

#include <math.h>

enum E_FILTER_TYPE
{
	E_LOW_PASS_2,
};

class FilterDesigner
{

public:
	FilterDesigner()
    {
		model = E_LOW_PASS_2;
    };

    ~FilterDesigner()
    {
    };

	void setParameter(float cut_off = 1200, float sample_rate = 44100, float Q = 0.707, float slope = 0, float magnitude = 0);
    void setCoefficients();
	float* getCoefficients();
	int model;

private:
	double TWO_PI = 6.283185307179586476925286766559;
	double EULER = 2.71828182845904523536;

	// numerator of transfer function
	float b0 = 0;
	float b1 = 0;
	float b2 = 0;
	// denominator of transfer function
    float a0 = 0;
    float a1 = 0;
    float a2 = 0;

	float omega;
	float sine_omega;
	float cosine_omega;
	float gain;
	float q;
	float slope;
	float alpha;
};

#endif /* FilterDesigner_h */