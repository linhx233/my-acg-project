#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "common.h"

class sphere: public hittable{
  public:
    sphere(const point3& center, double radius, shared_ptr<material> mat): 
    	center(ray(center,vec3(0,0,0))), radius(std::max(0.,radius)), mat(mat) {
			vec3 vecr(radius,radius,radius);
			boundingbox=bounding_box(center-vecr,center+vecr);
            area=4*pi*radius*radius;
		}
	sphere(const point3& center0, const point3& center1, double radius, shared_ptr<material> mat):
    	center(ray(center0,center1-center0)), radius(std::max(0.,radius)), mat(mat) {
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
    double sample_pdf(const ray& r)const override{
        hit_record rec; ray rt=r;
        double accum=0;
        while(hit(rt,interval(err,infty),rec)){
            double dist_squared=rec.t*rec.t*rt.direction().length_squared();
            double cos_t=abs(dot(rec.normal,normalize(rt.direction())));
            accum+=dist_squared/cos_t;
            rt=ray(rec.p,rt.direction(),rt.time());
        }
        return accum/area;
    }
    vec3 sample(const point3& origin, double time=0)const override{
        point3 P=center.at(time)+radius*random_unit_vector();
        return normalize(P-origin);
    }

  private:
    ray center;
    double radius;
    std::shared_ptr<material> mat;
	bounding_box boundingbox;
    double area;
	static void get_sphere_uv(const point3& p, double& u, double& v) {
        v=acos(-p.y())/pi;
        u=atan2(-p.z(),p.x())/(2*pi)+.5;
    }
};

class point: public hittable{
  public:
    point(const point3& P, shared_ptr<material> mat): P(P), S(make_shared<sphere>(P,1e-4,mat)){}

    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{return S->hit(r,ray_t,rec);}
    bounding_box bbox()const override{return S->bbox();}
    double sample_pdf(const ray& r)const override{S->sample_pdf(r);};
    vec3 sample(const point3& origin, const double time)const override{return S->sample(origin,time);}
  private:
    point3 P;
    shared_ptr<sphere> S;
};

#endif