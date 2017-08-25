#pragma once
//#include<stdio.h>
//#include<stdlib.h>
#ifdef _WINDOWS
#include<time.h>
#else
#include<sys/time.h>
#endif
#define bool	_Bool
#define false	0
#define true	1
int factorial(int num);
#define HUGE_VAL 1E+300 * 1E+10
enum Type
{
	Sin, Cos, Tan
};
_Bool CheckDegree(double degree, enum Type type);
double _sin(double t);
double _cos(double t);
double _tan(double t);
double sqrt_2(float a);
double sqrt_1(double a);
double __pow(double x, int y);
unsigned int _abs(int value);
double angle_to_radian(double degree, double min, double second);
void radian_to_angle(double rad, double ang[]);
int _floor(double a);
double _ceil(double x);
double _round(double val, int places);
double _exp(double x);
double _log10(double n);
double _log(double n);
double _log2(double n);
double _powf(double a, double x);
int __rand();
void srand_1(unsigned seed);
void srand_2();
double __asin(double x);
double __acos(double x);
double __atan(double x);
double __sinh(double x);
double __cosh(double x);
double __tanh(double x);
double __fmod(double _X, double _Y);
 double __fabs(double value);
 double F1(double x);
 double F2(double x);
 double simpson_2(double a, double b);
 double simpson_1(double a, double b,int flag);
 double asr_4(double a, double b, double eps, double A);
 double asr_1(double a, double b, double eps);
 double asr_2(double a, double b, double eps, double A,int flag);
 double asr_3(double a, double b, double eps,int flag);
 double eee(double x);
 double _Convert(double t);
 int __isnan(double d);
 int __isinf(double d);
 double ____atan2(double y, double x, int infNum);
 double __ldexp(double x, int exp);
 double __frexp(double x, int *exp);
