#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "pdf.h"

class material{
  public:
    virtual ~material()=default;
    virtual bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, 
						 ray& ray_out, double& sample_pdf)const{return 0;}
	virtual double bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const{return 0;}
    virtual color emit(const ray& ray_in, const hit_record& rec, 
					   double u, double v, const point3& p)const{return color(0,0,0);}
};

class lambertian: public material{
  public:
    lambertian(color albedo): tex(make_shared<solid_color>(solid_color(albedo))){}
    lambertian(const shared_ptr<texture>& tex): tex(tex){}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, 
				 ray& ray_out, double& sample_pdf)const override{
        vec3 scattered_direction=random_unit_vector()+rec.normal;
        while(scattered_direction.length_squared()<1e-12)
            scattered_direction=random_unit_vector()+rec.normal;
        ray_out=ray(rec.p,normalize(scattered_direction),ray_in.time());
        attenuation=tex->value(rec.u,rec.v,rec.p);
		sample_pdf=bsdf(ray_in,rec,ray_out);
        return 1;
    }
	double bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const override{
		return std::max(0.,dot(rec.normal,normalize(ray_out.direction())/pi));

	}
  private:
    shared_ptr<texture> tex;
};


class mirror: public material{
  public:
    mirror(color albedo): albedo(albedo){}
    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, 
				 ray& ray_out, double& sample_pdf)const override{
        ray_out=ray(rec.p,reflect(ray_in.direction(),rec.normal),ray_in.time());
        attenuation=albedo;
        return 1;
    }
  private:
    color albedo;
};

class metal: public material{
  public:
    metal(color albedo, double fuzz): albedo(albedo), fuzz(fuzz){}
    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, 
				 ray& ray_out, double& sample_pdf)const override{
        vec3 new_direction=normalize(reflect(ray_in.direction(),rec.normal))+random_unit_vector()*fuzz;
        ray_out=ray(rec.p,new_direction,ray_in.time());
        attenuation=albedo;
        return dot(new_direction,rec.normal)>0;
    }
  private:
    color albedo;
    double fuzz;
};

class dielectric: public material{
  public:
    dielectric(double refraction_rate, color albedo=color(1.0,1.0,1.0)):
        albedo(albedo),refraction_rate(refraction_rate){}
    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, 
				 ray& ray_out, double& sample_pdf)const override{
        attenuation=albedo;
        vec3 in_direction=normalize(ray_in.direction());
        double eta=rec.outer_face?1/refraction_rate:refraction_rate;
        ray_out=ray(rec.p,refract(in_direction,rec.normal,eta),ray_in.time());
        return 1;
    }
  private:
    color albedo;
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