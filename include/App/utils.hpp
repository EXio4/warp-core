#pragma once
#include <cstdlib>
#include <cstdint>

uint32_t rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255);

// https://stackoverflow.com/questions/828092/python-style-integer-division-modulus-in-c
static div_t div_floor(int x, int y) {
    div_t r = div(x, y);
    if (r.rem && (x < 0) != (y < 0)) {
        r.quot -= 1;
        r.rem  += y;
    }
    return r;
}

// https://www.ece.iastate.edu/~alexs/classes/2012_Fall_185/sample_exams/midterm2/2010/Lab_v1/3_BilinearInterpolation.c
inline double linearInterpolation(double x1, double f_x1, double x2, double f_x2, double x)
{
	double result = (x - x1)/(x2-x1)*f_x2  + (x2-x)/(x2-x1)*f_x1;
	return result;
}

inline double BilinearInterpolation(double q11, double q12, double q21, double q22, double x1, double y1, double x2, double y2, double x, double y) 
{
	double R1 = linearInterpolation(x1, q11, x2, q21, x);
	double R2 = linearInterpolation(x1, q12, x2, q22, x);
	return  linearInterpolation(y1, R1, y2, R2, y);
}