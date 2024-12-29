#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "interval.h"

#include <iostream>

using color = vec3;

inline double linear_to_srgb_channel(double x){
    if(x<0.0031308)return x*12.92;
    return 1.055*pow(x,1/2.4)-0.055;
}

inline color linear_to_srgb(color c){
    return color(linear_to_srgb_channel(c.e0),linear_to_srgb_channel(c.e1),linear_to_srgb_channel(c.e2));
}

inline double gamma_correction(double x){return pow(linear_to_srgb_channel(x),0.7);}

inline void write_color(std::ostream& out, const color& pixel_color){
    int rbyte(255.01*gamma_correction(interval::ratio.clamp(pixel_color.x())));
    int gbyte(255.01*gamma_correction(interval::ratio.clamp(pixel_color.y())));
    int bbyte(255.01*gamma_correction(interval::ratio.clamp(pixel_color.z())));
    out<<rbyte<<' '<<gbyte<<' '<<bbyte<<'\n';
}

inline color random_color(const double& min=0, const double& max=1){
    return color(random_double(min,max),random_double(min,max),random_double(min,max));
}


#endif