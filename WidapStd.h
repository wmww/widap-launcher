///standard Widap header (William's Interactive Development and Application Programing) header

#pragma once

#ifdef _WIN32 //if we are on a windows computer
	#include <windows.h>
#else //assumes we are on a linux computer
	#include <stdlib.h>
#endif

#include <math.h>

const double PI=3.14159265358979323846;

struct XYdbl //an XY cord made of doubles
{
	double x, y;
};

struct XYint //an XY cord made of ints
{
	int x, y;
};


///prototypes

//quickly uses the Pythagorean thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);
inline double dst(double num1, double num2);
inline double dst(XYdbl num1);
inline double dst(XYint num1);
inline double dst(XYdbl num1, XYdbl num2);
inline double dst(XYint num1, XYint num2);

//make and return an XY data type, should slowly be fazed out in favor of a template class
inline XYdbl mkXY(double x, double y);
inline XYint mkXY(int x, int y);
inline XYdbl mkXYdbl(XYint xyInt);
inline XYdbl mkXYdbl(double x, double y);
inline XYint mkXYint(XYdbl xyDbl);
inline XYint mkXYint(int x, int y);

//XY point operators
inline bool operator!=(XYint a, XYint b);
inline XYint operator+(XYint a, XYint b);

//degree and radian conversion
inline double deg2rad(double inpt);
inline double rad2deg(double inpt);

//as the first input goes from startI to endI, the output will go between startO and endO, output is not clamped
inline double grdnt(double inpt, double strtI, double endI, double strtO, double endO);

//return input if it is between low and hgh, otherwise returns low or hgh; low must be lower then hgh but this is not checked
inline double clamp(double inpt, double low, double hgh);
inline int clamp(int inpt, double low, double hgh);

//returns a random double in the specified range; is based on rand()
inline double drand(double low, double hgh); //this one may slow multi threading down
inline double drand(double low, double hgh, unsigned int * seed); //this one you must send the seed every time so it is faster for threads


///inline functions

inline double dst (double num1, double num2)
{
	//quickly uses the pathag thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);

    return sqrt(num1*num1+num2*num2);
}

inline double dst (XYdbl num1)
{
	//quickly uses the pathag thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);

    return dst(num1.x, num1.y);
}

inline double dst (XYint num1)
{
	//quickly uses the pathag thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);

    return dst(num1.x, num1.y);
}

inline double dst (XYdbl num1, XYdbl num2)
{
	//quickly uses the pathag thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);

    return dst(num1.x-num2.x, num1.y-num2.y);
}

inline double dst (XYint num1, XYint num2)
{
	//quickly uses the pathag thrum and returns the distance; ex: distance=dst(x1-x2, y1-y2);

    return dst(num1.x-num2.x, num1.y-num2.y);
}

inline XYdbl mkXY(double x, double y)
{
	XYdbl xy;
	
	xy.x=x;
	xy.y=y;
	
	return xy;
}

inline XYint mkXY(int x, int y)
{
	XYint xy;
	
	xy.x=x;
	xy.y=y;
	
	return xy;
}

inline XYdbl mkXYdbl(XYint xyInt)
{
	XYdbl xyDbl;
	
	xyDbl.x=xyInt.x;
	xyDbl.y=xyInt.y;
	
	return xyDbl;
}

inline XYdbl mkXYdbl(double x, double y)
{
	XYdbl xy;
	
	xy.x=x;
	xy.y=y;
	
	return xy;
}

inline XYint mkXYint(XYdbl xyDbl)
{
	XYint xyInt;
	
	xyInt.x=xyDbl.x;
	xyInt.y=xyDbl.y;
	
	return xyInt;
}

inline XYint mkXYint(int x, int y)
{
	XYint xy;
	
	xy.x=x;
	xy.y=y;
	
	return xy;
}

inline bool operator!=(XYint a, XYint b)
{
	return (a.x!=b.x || a.y!=b.y);
}

inline XYint operator+(XYint a, XYint b)
{
	a.x+=b.x;
	a.y+=b.y;
	
	return a;
}

inline double deg2rad(double inpt)
{
	//converts degrees to radians

	return inpt*PI/180;
}

inline double rad2deg(double inpt)
{
	//converts radians to degrees

	return inpt*180/PI;
}

inline double grdnt(double inpt, double strtI, double endI, double strtO, double endO)
{
	//this function returns a number between strtO and endO in proportion to where inpt is between strtI and endI

	if (strtI==endI) //stop divide by 0
		return endO;
	else
		return ((inpt-strtI)*(strtO-endO))/(strtI-endI)+strtO;
}

inline double clamp(double inpt, double low, double hgh)
{
	if (inpt>=low && inpt<=hgh)
		return inpt;
	else if (inpt>hgh)
		return hgh;
	else
		return low;
}

inline int clamp(int inpt, double low, double hgh)
{
	if (inpt>=low && inpt<=hgh)
		return inpt;
	else if (inpt>hgh)
		return hgh;
	else
		return low;
}

inline double drand(double low, double hgh)
{
	//uses rand() but returns a double between low and high
	if (low==hgh)
		return low;

	else
		return (double)rand()/(RAND_MAX/(hgh-low))+low;
}

inline double drand(double low, double hgh, unsigned int * seed)
{
	//uses rand() but returns a double between low and high
	
	if (low==hgh)
		return low;
	
	else
		return (double)rand_r(seed)/(RAND_MAX/(hgh-low))+low;
}

