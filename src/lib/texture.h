#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include<memory>
#include "rtw_stb_image.h"
#include "perlin.h"

class texture{
  public:
    virtual ~texture()=default;
    virtual color value(const point2& tex_coord, const point3& p)const=0;
};


class solid_color: public texture{
  public:
    solid_color(const color& albedo): albedo(albedo){}
    solid_color(double r, double g, double b): solid_color(color(r,g,b)){}

    color value(const point2& tex_coord, const point3& p)const override{ return albedo;}

  private:
    color albedo;
};

class checker_texture: public texture{
  public:
    checker_texture(double scale, shared_ptr<texture> texture_even, shared_ptr<texture> texture_odd):
        inv_scale(1/scale), texture_even(texture_even), texture_odd(texture_odd){}
    checker_texture(double scale, const color& c_even, const color& c_odd):
        checker_texture(scale, make_shared<solid_color>(solid_color(c_even)), make_shared<solid_color>(solid_color(c_odd))){}
    
    color value(const point2& tex_coord, const point3& p)const override{
        int xx=floor(p.x()*inv_scale),yy=floor(p.y()*inv_scale),zz=floor(p.z()*inv_scale);
        return (xx+yy+zz)%2==0?texture_even->value(tex_coord,p):texture_odd->value(tex_coord,p);
    }
  private:
    double inv_scale;
    shared_ptr<texture> texture_even,texture_odd;
};

class image_texture: public texture{
  public:
    image_texture(const char* filename): image(filename){}
    image_texture(const std::string filename): image(filename.c_str()){}
	image_texture(const aiTexture* tex): image(tex){}
	
    color value(const point2& tex_coord, const point3& p)const override{
        if(image.height()<=0)return color(0,1,1);
        double u=tex_coord.u,v=tex_coord.v;
		    u=u>1?1:(u<0?0:u),v=v>1?1:(v<0?0:v);
        int i=u*image.width(),j=(1-v)*image.height();
        auto pixel=image.pixel_data(i,j);
        return color(pixel[0],pixel[1],pixel[2])/255;
    }
  private:
    rtw_image image;
};

class grey_noise_texture: public texture{
  public:
    grey_noise_texture(double freq): freq(freq){}

    color value(const point2& tex_coord, const point3& p)const override{
        return color(1,1,1)*(0.5+0.5*sin(freq*p.z()+10*noise.turb(p,7)));
    }
  private:
    double freq;
    perlin_noise noise;
};

#endif