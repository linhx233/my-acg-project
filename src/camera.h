#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "hittable.h"
#include "material.h"

class camera{
  public:
    double aspect_ratio;
    int image_width;
    int samples_per_pixel;
    int max_depth;

    double vfov;
    point3 position;
    point3 lookat;
    vec3 vup;
    double focus_dist;
    double defocus_angle;

    camera(int image_width=100, 
           double aspect_ratio=1.0, 
           int samples_per_pixel=10, 
           int max_depth=10, 
           double vfov=90,
           point3 position=point3(0,0,0),
           point3 lookat=point3(0,0,-1),
           vec3 vup=vec3(0,1,0),
           double focus_dist=10,
           double defocus_angle=0): 
        image_width(image_width), 
        aspect_ratio(aspect_ratio), 
        samples_per_pixel(samples_per_pixel),
        max_depth(max_depth),
        vfov(vfov),
        position(position),
        lookat(lookat),
        vup(vup),
        focus_dist(focus_dist),
        defocus_angle(defocus_angle){}

    void render(const hittable& scene){
        init();
        
        std::cout<<"P3\n"<<image_width<<' '<<image_height <<"\n255\n";

        for (int j=0;j<image_height;j++){
            std::clog<<"\rScanlines remaining: "<<(image_height-j)<<' '<<std::flush;
            for (int i=0;i<image_width;i++){
                point3 pixel_center=pixel00_loc+j*pixel_delta_v+i*pixel_delta_u;
                vec3 ray_direction=normalize(pixel_center-center);
                color pixel_color(0,0,0);
                for(int T=0;T<samples_per_pixel;T++){
                    point3 pixel_sample=pixel_center+random_double(-0.5,0.5)*pixel_delta_u
                                                    +random_double(-0.5,0.5)*pixel_delta_v;
                    point3 ray_origin=sample_in_defocus_disk();
                    double ray_time=random_double();
                    ray r(ray_origin,normalize(pixel_sample-ray_origin),ray_time);

                    pixel_color+=ray_color(r,scene,max_depth)/samples_per_pixel;
                }
                write_color(std::cout,pixel_color);
            }
        }
        std::clog << "\rDone.                 \n";
    }
  private:
    int image_height;
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u, pixel_delta_v;//u horizontal, v vertical
    vec3 u,v,w;
    vec3 defocus_u,defocus_v;
    
    void init(){
        image_height=std::max(1.,image_width/aspect_ratio);

        center=position;

        double theta=deg_to_rad(vfov);
        double h=tan(theta/2);
        double viewport_height=focus_dist*h*2;
        double viewport_width=viewport_height*(double(image_width)/image_height);

        w=normalize(position-lookat);
        u=normalize(cross(vup,w));
        v=cross(w,u);

        vec3 viewport_u=viewport_width*u;
        vec3 viewport_v=-viewport_height*v;
        pixel_delta_u=viewport_u/image_width;
        pixel_delta_v=viewport_v/image_height;

        vec3 viewport_upper_left=center-focus_dist*w-viewport_u/2-viewport_v/2;
        pixel00_loc=viewport_upper_left+0.5*(pixel_delta_u+pixel_delta_v);

        double defocus_radius=focus_dist*tan(deg_to_rad(defocus_angle)/2);
        defocus_u=defocus_radius*u,defocus_v=defocus_radius*v;
    }

    vec3 sample_in_defocus_disk(){
        double x=random_double(-1,1),y=random_double(-1,1);
        while(x*x+y*y>=1.0)x=random_double(-1,1),y=random_double(-1,1);
        return center+x*defocus_u+y*defocus_v;
    }
    
    color ray_color(const ray& r,const hittable& obj,const int& depth){
        if(depth<=0)return color(0,0,0);
        hit_record rec;
        if(obj.hit(r,interval(err,infty),rec)){
            color attenuation;
            ray scattered;
            if(rec.mat->scatter(r,rec,attenuation,scattered))
                return attenuation*ray_color(scattered,obj,depth-1);
            return color(0,0,0);
        }
    
        vec3 unit_direction = normalize(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};

#endif 