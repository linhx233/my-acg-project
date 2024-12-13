#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "hittable.h"
#include "material.h"
#include "pdf.h"
#include "scatter_record.h"

#include<thread>
#include<mutex>

class camera{
  public:
	double aspect_ratio;
	int image_width;
	int samples_per_pixel;
	int max_depth;
	color background;

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
		   color background=color(0,0,0),
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
		background(background),
		vfov(vfov),
		position(position),
		lookat(lookat),
		vup(vup),
		focus_dist(focus_dist),
		defocus_angle(defocus_angle){}

	void render(const hittable& scene, const hittable& lights=hittable_list()){
		init();
		
		std::cout<<"P3\n"<<image_width<<' '<<image_height <<"\n255\n";

		int sqrt_spp=ceil(sqrt(samples_per_pixel));
		int real_spp=sqrt_spp*sqrt_spp;
		int num_thread=16;
		std::thread *th = new std::thread[num_thread];
		int scanlines_remaining=image_height;
		std::mutex mtx;
		color *pixel_colors = new color[image_width*image_height];

		auto subprocess = [&](int r) -> void{
			color *pixel_color_buffer = new color[real_spp];
			for (int j=r;j<image_height;j+=num_thread){
				mtx.lock();
				std::clog<<"\rScanlines remaining: "<<scanlines_remaining<<' '<<std::flush;
				scanlines_remaining--;
				mtx.unlock();
				for (int i=0;i<image_width;i++){
					point3 pixel_upper_left=viewport_upper_left+j*pixel_delta_v+i*pixel_delta_u;
					color pixel_color(0,0,0);
					for(int si=0;si<sqrt_spp;si++)
						for(int sj=0;sj<sqrt_spp;sj++){
							point3 pixel_sample=pixel_upper_left+(si+random_double())/sqrt_spp*pixel_delta_u
																+(sj+random_double())/sqrt_spp*pixel_delta_v;
							point3 ray_origin=sample_in_defocus_disk();
							double ray_time=random_double();
							ray r(ray_origin,normalize(pixel_sample-ray_origin),ray_time);

							color raycolor=ray_color(r,scene,lights,1./max_depth,max_depth);
							if(raycolor.e0!=raycolor.e0)raycolor.e0=0;
							if(raycolor.e1!=raycolor.e1)raycolor.e1=0;
							if(raycolor.e2!=raycolor.e2)raycolor.e2=0;
							pixel_color_buffer[si*sqrt_spp+sj]=raycolor;
						}
					pixel_colors[j*image_width+i]=anti_aliasing(pixel_color_buffer,sqrt_spp,sqrt_spp,
																std::min(8,int(sqrt(sqrt_spp))));
				}
			}
			delete[] pixel_color_buffer;
		};

		for(int i=0;i<num_thread;i++)th[i]=std::thread(subprocess,i);
		for(int i=0;i<num_thread;i++)th[i].join();
		for(int i=0;i<image_height*image_width;i++)
			write_color(std::cout,pixel_colors[i]);
		delete[] pixel_colors;
		delete[] th;
		std::clog << "\rDone.                 \n";
	}
  private:
	int image_height;
	point3 center;
	point3 pixel00_loc,viewport_upper_left;
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

		viewport_upper_left=center-focus_dist*w-viewport_u/2-viewport_v/2;
		pixel00_loc=viewport_upper_left+0.5*(pixel_delta_u+pixel_delta_v);

		double defocus_radius=focus_dist*tan(deg_to_rad(defocus_angle)/2);
		defocus_u=defocus_radius*u,defocus_v=defocus_radius*v;
	}

	vec3 sample_in_defocus_disk(){
		double x=random_double(-1,1),y=random_double(-1,1);
		while(x*x+y*y>=1.0)x=random_double(-1,1),y=random_double(-1,1);
		return center+x*defocus_u+y*defocus_v;
	}
	
	color ray_color(const ray& r, const hittable& obj, const hittable& lights, const double p, const int depth){
		if(depth<=0)return color(0,0,0);
		hit_record rec;
		if(!obj.hit(r,interval(err,infty),rec))return background;
		scatter_record scatter;
		ray scattered_ray;
		color emitted=rec.mat->emit(r,rec);
		if(depth==1)return emitted;
		double bsdf;
		if(!rec.mat->scatter(r,rec,scatter))return emitted;
		if(!scatter.using_importance_sampling){
			scattered_ray=scatter.sample_ray;
			bool extra_bounce=(scattered_ray.direction()==r.direction());
			return scatter.attenuation*ray_color(scattered_ray,obj,lights,p,depth-1+extra_bounce)+emitted;
		}
		auto light_pdf=make_shared<directed_pdf>(lights, rec.p);
		auto surface_pdf=scatter.sample_pdf;
		if(random_double()<=std::max(p,pow(depth,-1.66667))){//Russian Roulette
			const int num_sample=5;
			color accum(0,0,0);
			for(int T=0;T<num_sample;T++){
				scattered_ray=ray(rec.p,light_pdf->sample(),r.time());
				bsdf=rec.mat->bsdf(r,rec,scattered_ray);
				accum+=bsdf*ray_color(scattered_ray,obj,lights,p,1)/num_sample;
			}
			return scatter.attenuation*accum+emitted;
		}
		//MIS with power-2 heuristic
		const int num_sample_light=3;
		double w_surface,w_light,w;
		color accum(0,0,0);
		scattered_ray=ray(rec.p,surface_pdf->sample(),r.time());
		bsdf=rec.mat->bsdf(r,rec,scattered_ray);
		if(bsdf>=err){
			w_surface=surface_pdf->value(scattered_ray.direction());
			w_light=light_pdf->value(scattered_ray.direction());
			w=square(w_surface)/(square(w_surface)+square(num_sample_light*w_light));
			accum+=bsdf*w*ray_color(scattered_ray,obj,lights,p,depth-1)/w_surface;
		}
		for(int T=0;T<num_sample_light;T++){
			scattered_ray=ray(rec.p,light_pdf->sample(),r.time());
			bsdf=rec.mat->bsdf(r,rec,scattered_ray);
			if(bsdf<err)continue;
			w_light=light_pdf->value(scattered_ray.direction());
			if(w_light<err)continue;
			w_surface=surface_pdf->value(scattered_ray.direction());
			w=square(num_sample_light*w_light)/(square(w_surface)+square(num_sample_light*w_light));
			accum+=bsdf*w*ray_color(scattered_ray,obj,lights,p,1)/(w_light*num_sample_light);
		}
		return scatter.attenuation*accum+emitted;
	}

	color anti_aliasing(color* buffer, int row, int col, int pool_num){
		int pool_num_r=std::min(pool_num,row),pool_num_c=std::min(pool_num,col);
		color res(0,0,0);
		for(int i=0;i<pool_num_r;i++)
			for(int j=0;j<pool_num_c;j++){
				int lr=1ll*i*row/pool_num_r,rr=1ll*(i+1)*row/pool_num_r;
				int lc=1ll*j*col/pool_num_c,rc=1ll*(j+1)*col/pool_num_c;
				color tmp(0,0,0);
				for(int si=lr;si<rr;si++)
					for(int sj=lc;sj<rc;sj++)
						tmp+=buffer[si*col+sj];
				tmp/=1ll*(rr-lr)*(rc-lc);
				if(tmp.e0>1)tmp.e0=1;
				if(tmp.e1>1)tmp.e1=1;
				if(tmp.e2>1)tmp.e2=1;
				res+=tmp*(rr-lr)*(rc-lc);
			}
		return res/(row*col);
	}
};

#endif 