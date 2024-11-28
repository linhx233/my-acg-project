#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include<memory>

using std::shared_ptr;
using std::make_shared;

class texture{
  public:
    virtual ~texture()=default;
    virtual color value(double u, double v, const point3& p)const=0;
};


class solid_color: public texture{
  public:
    solid_color(const color& albedo): albedo(albedo){}
    solid_color(double r, double g, double b): solid_color(color(r,g,b)){}

    color value(double u, double v, const point3& p)const override{ return albedo;}

  private:
    color albedo;
};

class checker_texture: public texture{
  public:
    checker_texture(double scale, shared_ptr<texture> texture_even, shared_ptr<texture> texture_odd):
        inv_scale(1/scale), texture_even(texture_even), texture_odd(texture_odd){}
    checker_texture(double scale, const color& c_even, const color& c_odd):
        checker_texture(scale, make_shared<solid_color>(solid_color(c_even)), make_shared<solid_color>(solid_color(c_odd))){}
    
    color value(double u,double v, const point3& p)const override{
        int xx=floor(p.x()*inv_scale),yy=floor(p.y()*inv_scale),zz=floor(p.z()*inv_scale);
        return (xx+yy+zz)%2==0?texture_even->value(u,v,p):texture_odd->value(u,v,p);
    }
  private:
    double inv_scale;
    shared_ptr<texture> texture_even,texture_odd;
};

#endif