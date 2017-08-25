#include "lmathlibc.h"
#define PI 3.1415926535897932846
#define PI_DIV_2  PI/2
#define E    2.7182818284590452354 
#define SpaceDegree 180
#define NothingnessSin1 180
#define NothingnessCos1 90
#define NothingnessTan1 NothingnessCos1
int factorial(int num){
	int a = 1;
	for (int i = 1; i<num; i++){
		a = a*(i + 1);
	}
	return a;
}
_Bool CheckDegree(double degree,enum Type type)
{
	double temp;
	int i;
	switch (type)
	{
	case Sin:
		temp = __fabs(degree) - NothingnessSin1;
		i =(int)(temp / SpaceDegree);
		if (temp == (i*SpaceDegree))
		{
			return 1;
		}
		return 0;
	case Cos:
		temp = __fabs(degree) - NothingnessCos1;
		i = (int)(temp / SpaceDegree);
		if (temp == (i*SpaceDegree))
		{
			return 1;
		}
		return 0;
	case Tan:
		temp = __fabs(degree) - NothingnessTan1;
		i = (int)(temp / SpaceDegree);
		if (temp == (i*SpaceDegree))
		{
			return 1;
		}
		return 0;
	default:
		return 0;
	}
}
double __pow(double x, int y)
{
	if (y==0)
	{
		return 1;
	}
	else if (y < 0)
	{
		return 1 / __pow(x, -y);
	}
	else
	{
		if (y == 1) return x;
		double result = 0;
		double tmp = __pow(x, y / 2);
		if ((y & 1) != 0) //奇数
		{
			result = x * tmp * tmp;
		}
		else
		{
			result = tmp * tmp;
		}

		return result;
	}
}
double _sin(double t){
	if (CheckDegree(t, Sin))
	{
		return 0.0;
	}
	bool flag = true;//T减法，F加法
	t = PI / (180 / t);//换算成弧度
	double result = t;
	double a = 0, b = 0;
	int oddnum = 0;
	for (int i = 0; i<12; i++){//精度
		oddnum = 2 * (i + 1) + 1;
		a = __pow(t, oddnum);//
		b = factorial(oddnum);
		if (flag){
			result = result - (a / b);
			flag = false;
		}
		else{
			result = result + (a / b);
			flag = true;
		}
	}
	return result;
}
double _cos(double t)
{
	if (CheckDegree(t, Cos))
	{
		return 0.0;
	}
	double temp1, temp2, temp3,temp4;
	temp1 = _floor(t);
	temp2 = (t - temp1)*60;
	temp3 = _floor(temp2);
	temp4 = (temp2 - temp3)*60;
	double result = angle_to_radian(temp1, temp3,temp4);
	double result2[3];
	radian_to_angle(PI_DIV_2 - result, result2);
	double result3 = (result2[0]+result2[1]/60+result2[2]/3600);
	return(_sin(result3));
	//return sqrt(1 - __pow(_sin(t), 2));
}
double _tan(double t)
{
	if (CheckDegree(t, Tan))
	{
		return HUGE_VAL;
	}
	return _sin(t) / _cos(t);
}
double sqrt_2(float a)
{
	double x,y;
	x = 0.0;
	y = a / 2;
	while (x != y)
	{
		x = y;
		y = (x + a / x) / 2;
	}
	return x;
}
double sqrt_1(double a)
{
	double x, y;
	x = 0.0;
	y = a / 2;
	while (x != y)
	{
		x = y;
		y = (x + a / x) / 2;
	}
	return x;
}
//角度转换为弧度
double angle_to_radian(double degree, double min, double second)
{
	double flag = (degree < 0) ? -1.0 : 1.0;			//判断正负
	if (degree<0)
	{
		degree = degree * (-1.0);
	}
	double angle = degree + min / 60 + second / 3600;
	double result = flag * (angle * PI) / 180;
	return result;
}
//弧度转换为角度
void radian_to_angle(double rad, double ang[])
{
	double flag = (rad < 0) ? -1.0 : 1.0;
	if (rad<0)
	{
		rad = rad * (-1.0);
	}
	double result = (rad * 180) / PI;
	double degree = (int)(result);
	double min = (result - degree) * 60;
	double second = (min - (int)(min)) * 60;
	ang[0] = flag * degree;
	ang[1] = min;
	ang[2] = second;
}
int _floor(double a) {
	int b = 0;
	if (a >= 0) {
		b = (int)a;
	}
	else {
		b = (int)a - 1;
	} 
	return b;
}
double _ceil(double x)
{
	double y = x;
	if ((*(((int *)&y) + 1) & 0x80000000) != 0)//或者if(x<0)
	{
		return (float)((int)x);
	}
	else                 //讨论非负的情况。
	{
		if (x == 0)
		{
			return (float)((int)x);
		}
		else
		{
			return (float)((int)x) + 1;
		}
	}
}//向上取整

double _round(double val, int places) {
	double t;
	double f = __pow(10.0, places);
	double x = val * f;

	if (__isinf(x) || __isnan(x)) {
		return val;
	}

	if (x >= 0.0) {
		t = _ceil(x);
		if ((t - x) > 0.50000000001) {
			t -= 1.0;
		}
	}
	else {
		t = _ceil(-x);
		if ((t + x) > 0.50000000001) {
			t -= 1.0;
		}
		t = -t;
	}
	x = t / f;

	return !__isnan(x) ? x : t;
}
double _exp(double x) {
	if (x<0)
	{
		return 1 / _exp(-x);
	}
	int n = _floor(x);
	x -= n;
	double e1 = __pow(E, n);
	double e2 = eee(x);
	return e1*e2;
}
double eee(double x)
{
	if (x>1e-3)
	{
		double ee = eee(x / 2);
		return ee*ee;
	}
	return 1 + x + x*x / 2 + __pow(x, 3) + __pow(x, 4) / 24 + __pow(x, 5) / 120;
}
double _log10(double n)
{
	return (_log(n) / _log(10));
}
double  _log(double n)
{
	return asr_1(1, n, 1e-8);
}
double  _log2(double n)
{
	return (_log(n) / _log(2));
}
double F1(double x)
{
	return 1 / x;
}
double F2(double x)
{
	return 1 / sqrt_1(1 - x*x);
}
double simpson_2(double a, double b)
{
	return simpson_1(a, b, 1);
}
double simpson_1(double a, double b, int flag)
{
	double c = a + (b - a) / 2;
	if (flag==1)
	{
     return (F1(a) + 4 * F1(c) + F1(b))*(b - a) / 6;
	}
	if (flag==2)
	{
		return (F2(a) + 4 * F2(c) + F2(b))*(b - a) / 6;
	}
	return -0.0;
}
double asr_4(double a, double b, double eps, double A)
{
	double c = a + (b - a) / 2;
	double L = simpson_2(a, c), R = simpson_2(c, b);
	if (__fabs(L+R-A)<=15*eps)
	{
		return L + R + (L + R - A) / 15.0;
	}
	return asr_4(a, c, eps / 2, L) + asr_4(c, b, eps / 2, R);
}
double asr_2(double a, double b, double eps, double A,int flag)
{
	double c = a + (b - a) / 2;
	double L = simpson_1(a, c,flag), R = simpson_1(c, b,flag);
	if (__fabs(L + R - A) <= 15 * eps)
	{
		return L + R + (L + R - A) / 15.0;
	}
	return asr_2(a, c, eps / 2, L,flag) + asr_2(c, b, eps / 2, R,flag);
}
double asr_1(double a, double b, double eps)
{
	return asr_4(a, b, eps, simpson_2(a, b));
}
double asr_3(double a, double b, double eps,int flag)
{
	return asr_2(a, b, eps, simpson_1(a, b,flag),flag);
}
double __asin(double x)
{
	if (__fabs(x)>1)
	{
		return -1;
	}
	double fl = 1.0;
	if (x<0)
	{
		fl = -1;
		x *= -1;
	}
	if (__fabs(x-1)<1e-7)
	{
		return PI_DIV_2;
	}
	return (fl*asr_3(0, x, 1e-8, 2));
}
unsigned long next = 1;
double __acos(double x)
{
	if (__fabs(x)>1)
	{
		return - 1;
	}
	return PI_DIV_2 - __asin(x);
}
double __atan(double x)
{
	if (x<0)
	{
		return - __atan(x);
	}
	if (x>1)
	{
		return PI_DIV_2 - __atan(1 / x);
	}
	if (x>1e-3)
	{
		return 2 * __atan((sqrt_1(1 + x*x) - 1) / x);
	}
	return x - __pow(x, 3) / 3 + __pow(x, 5) / 5 - __pow(x, 7) / 7 + __pow(x, 9) / 9;
}
double __sinh(double x)
{
	return (_exp(x) - _exp(-x)) / 2;
}
double __cosh(double x)
{
	return (_exp(x) + _exp(-x)) / 2;
}
double __tanh(double x)
{
	double ext = _exp(x);
	return 1 - 2 / (ext*ext + 1);
}
/* RAND_MAX assumed to be 32767 */
int __rand() {
	next = next * 1103515245 + 0x77fdff5;
	return((unsigned)(next / 65536) % 32768);
}
void srand_2()
{
	srand_1((unsigned int)time(NULL));
}
void srand_1(unsigned seed) {
	next = seed;
}
double _Convert(double t)
{
	double temp1, temp2, temp3, temp4;
	temp1 = _floor(t);
	temp2 = (t - temp1) * 60;
	temp3 = _floor(temp2);
	temp4 = (temp2 - temp3) * 60;
	double result = angle_to_radian(temp1, temp3, temp4);
	return result;
}
double __fmod(double _X, double _Y)
{
	return _X - (int)(_X / _Y) * _Y;
}
int __isnan(double d)
{
	union
	{
		unsigned long long l;
		double d;
	} u;
	u.d = d;
	return (u.l == 0x7FF8000000000000ll || u.l == 0x7FF0000000000000ll || u.l == 0xfff8000000000000ll);
}
int __isinf(double d)
{
	union
	{
		unsigned long long l;
		double d;
	} u;
	u.d = d;
	return (u.l == 0x7FF0000000000000ll ? 1 : u.l == 0xFFF0000000000000ll ? -1 : 0);
}
double ____atan2(double y, double x, int infNum)
{
	int i;
	double z = y / x, sum = 0.0f, temp;
	double del = z / infNum;

	for (i = 0; i < infNum; i++)
	{
		z = i*del;
		temp = 1 / (z*z + 1) * del;
		sum += temp;
	}

	if (x>0)
	{
		return sum;
	}
	else if (y >= 0 && x < 0)
	{
		return sum + PI;
	}
	else if (y < 0 && x < 0)
	{
		return sum - PI;
	}
	else if (y > 0 && x == 0)
	{
		return PI / 2;
	}
	else if (y < 0 && x == 0)
	{
		return -1 * PI / 2;
	}
	else
	{
		return 0;
	}
}
double __ldexp(double x, int exp)
{
	return x*__pow(2, exp);
}
double __frexp(double x, int *exp)
{
	if (x == 0)
	{
		*exp = 0;
		return 0;
	}
	*exp=(int)_log2(x);
	return (x - __pow(2.0,*exp));
}
double __fabs(double value)
{
	return value > 0 ? value : (-value);
}
double _powf(double a, double x)
{
	return _exp(x*_log(a));
}
unsigned int _abs(int value)
{
	unsigned int copyed_value = value;
	return (copyed_value > 0x80000000) ? -value : copyed_value;
}
