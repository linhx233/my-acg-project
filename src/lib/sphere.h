#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "common.h"

class sphere: public hittable{
  public:
    sphere(const point3& center, double radius, std::shared_ptr<material> mat): 
    	center(ray(center,vec3(0,0,0))), radius(std::fmax(0,radius)), mat(mat) {
			vec3 vecr(radius,radius,radius);
			boundingbox=bounding_box(center-vecr,center+vecr);
		}
	sphere(const point3& center0, const point3& center1, double radius, std::shared_ptr<material> mat):
    	center(ray(center0,center1-center0)), radius(std::fmax(0,radius)), mat(mat) {
			vec3 vecr(radius,radius,radius);
			boundingbox=bounding_box(bounding_box(center0-vecr,center0+vecr),
									 bounding_box(center1-vecr,center1+vecr));
		}

    inline vec3 normAt(const point3& p, double time)const{ return (p-center.at(time))/radius;}
    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        vec3 oc=center.at(r.time())-r.origin();
        double a=r.direction().length_squared();
        double b=dot(r.direction(),oc);
        double c=oc.length_squared()-radius*radius;

        double delta=b*b-a*c;
        if(delta<0)return 0;
        double sqrtd=std::sqrt(delta);
        double t=(b-std::sqrt(delta))/a;
        if(!ray_t.surrounds(t)){
            t=(b+sqrtd)/a;
            if(!ray_t.surrounds(t))return 0;
        }

        rec.t=t;
        rec.p=r.at(t);
		vec3 outer_norm=normAt(rec.p,r.time());
        rec.set_normal(r,outer_norm);
		get_sphere_uv(outer_norm,rec.u,rec.v);
        rec.mat=mat;
        return 1;
    }
	bounding_box bbox()const override{ return boundingbox;}

  private:
    ray center;
    double radius;
    std::shared_ptr<material> mat;
	bounding_box boundingbox;
	static void get_sphere_uv(const point3& p, double& u, double& v) {
        v=acos(-p.y())/pi;
        u=atan2(-p.z(),p.x())/(2*pi)+.5;
    }
};

#endif