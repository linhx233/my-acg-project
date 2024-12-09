#ifndef COMMON_H
#define COMMON_H

#include<iostream>
#include<cmath>
#include<cstdlib>
#include<float.h>
#include<memory>
#include<string>

using std::shared_ptr;
using std::make_shared;

const double infty=1e100;
const double pi=3.1415926535897932384626433832795;
const double err=1e-6;

inline double random_double(){ return (std::rand()/(RAND_MAX+1.)+std::rand())/(RAND_MAX+1.);}
inline double random_double(double min, double max){ return min+(max-min)*random_double();}

inline double deg_to_rad(double x){ return x/180*pi;}

inline double square(double x){ return x*x;}

inline int argmax(int a,int b,int c){ return a>=b?(a>=c?0:2):(b>=c?1:2);}

template<typename T>
T linear_interpolate(T a[2], double u){
    return (1-u)*a[0]+u*a[1];
}

template<typename T>
T bilinear_interpolate(T a[2][2], double u, double v){
    return (1-u)*((1-v)*a[0][0]+v*a[0][1])+u*((1-v)*a[1][0]+v*a[1][1]);
}

template<typename T>
T trilinear_interpolate(T a[2][2][2], double u, double v,double w){
    return (1-u)*((1-v)*((1-w)*a[0][0][0]+w*a[0][0][1])+v*((1-w)*a[0][1][0]+w*a[0][1][1]))
           +u*((1-v)*((1-w)*a[1][0][0]+w*a[1][0][1])+v*((1-w)*a[1][1][0]+w*a[1][1][1]));
}


#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "interval.h"
#include "mat3.h"
#include "orthonormal_basis.h"

#endif