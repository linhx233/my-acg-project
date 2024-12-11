#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "pdf.h"
#include "scatter_record.h"

class material{
  public:
    virtual ~material()=default;
    virtual bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const{return 0;}
	virtual double bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const{return 0;}
    virtual color emit(const ray& ray_in, const hit_record& rec, 
					   double u, double v, const point3& p)const{return color(0,0,0);}
};

class lambertian: public material{
  public:
    lambertian(color albedo): tex(make_shared<solid_color>(solid_color(albedo))){}
    lambertian(const shared_ptr<texture>& tex): tex(tex){}

    bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
        vec3 scattered_direction=random_unit_vector()+rec.normal;
        while(scattered_direction.length_squared()<1e-12)
            scattered_direction=random_unit_vector()+rec.normal;
        scatter.attenuation=tex->value(rec.u,rec.v,rec.p);
		scatter.using_importance_sampling=true;
		scatter.sample_pdf=make_shared<lambertian_pdf>(rec.normal);
        return 1;
    }
	double bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const override{
		return std::max(0.,dot(rec.normal,normalize(ray_out.direction())/pi));

	}
  private:
    shared_ptr<texture> tex;
};

class metal: public material{
  public:
    metal(color albedo, double fuzz): tex(make_shared<solid_color>(albedo)), fuzz(std::min(fuzz,1.)){}
	metal(const shared_ptr<texture>& tex, double fuzz): tex(tex), fuzz(std::min(fuzz,1.)){}
    bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
        vec3 new_direction=normalize(reflect(ray_in.direction(),rec.normal))+random_unit_vector()*fuzz;
        scatter.attenuation=tex->value(rec.u,rec.v,rec.p);
		scatter.using_importance_sampling=false;
		scatter.sample_ray=ray(rec.p,new_direction,ray_in.time());
        return dot(new_direction,rec.normal)>0;
    }
  private:
    shared_ptr<texture> tex;
    double fuzz;
};

class dielectric: public material{
  public:
    dielectric(double refraction_rate, color albedo=color(1.0,1.0,1.0)):
        tex(make_shared<solid_color>(albedo)),refraction_rate(refraction_rate){}
    bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
        vec3 in_direction=normalize(ray_in.direction());
        double eta=rec.outer_face?1/refraction_rate:refraction_rate;
        scatter.attenuation=tex->value(rec.u,rec.v,rec.p);
		scatter.using_importance_sampling=false;
        scatter.sample_ray=ray(rec.p,refract(in_direction,rec.normal,eta),ray_in.time());
        return 1;
    }
  private:
    shared_ptr<texture> tex;
    double refraction_rate;
};

class diffuse_light: public material {
  public:
    diffuse_light(shared_ptr<texture> tex): tex(tex){}
    diffuse_light(const color& emit): tex(make_shared<solid_color>(emit)){}

    color emit(const ray& ray_in, const hit_record& rec,double u, double v, const point3& p)const override{ 
		return rec.outer_face?tex->value(u, v, p):color(0,0,0);
	}
  private:
    shared_ptr<texture> tex;
};

#endif