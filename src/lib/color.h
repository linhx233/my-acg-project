#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "interval.h"

#include <iostream>

using color = vec3;

inline double gamma_correction(double x){return sqrt(x);}

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