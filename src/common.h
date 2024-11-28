#ifndef COMMON_H
#define COMMON_H

#include<iostream>
#include<cmath>
#include<cstdlib>
#include<float.h>

using std::shared_ptr;
using std::make_shared;

const double infty=DBL_MAX;
const double pi=3.1415926535897932384626433832795;
const double err=1e-6;

inline double random_double(){ return std::rand()/(RAND_MAX+1.);}
inline double random_double(double min, double max){ return min+(max-min)*random_double();}

inline double deg_to_rad(double x){ return x/180*pi;}

inline double square(double x){ return x*x;}

inline int argmax(int a,int b,int c){ return a>=b?(a>=c?0:2):(b>=c?1:2);}

#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "interval.h"

#endif