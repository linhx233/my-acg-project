#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"
#include "common.h"
#include "mat3.h"

class quad: public hittable{
  public:
    quad(const point3& Q, const vec3& u, const vec3& v, const shared_ptr<material>& mat): 
        Q(Q), u(u), v(v), mat(mat){ 
        boundingbox=bounding_box(bounding_box(Q,Q+u+v),bounding_box(Q+u,Q+v));
        normal=normalize(cross(u,v));
        area=length(cross(u,v));
    }
    vec3 norm(){return normal;}
    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        mat3 A(-r.direction(),u,v);
        vec3 b=r.origin()-Q;
        if(abs(det(A))<err)return 0;
        vec3 v=inv(A)*b;
        if(!interval::ratio.contains(v[1])||!interval::ratio.contains(v[2])||!ray_t.contains(v[0]))return 0;
        rec.t=v[0];
        rec.p=r.at(rec.t);
        rec.set_normal(r,normal);
        rec.mat=mat;
        rec.u=v[1],rec.v=v[2];
        return 1;
    }
    bounding_box bbox()const override{ return boundingbox;}
    double sample_pdf(const ray& r)const override{
        hit_record rec;
        if(!hit(r,interval(err,infty),rec))return 0;
        double dist_squared=rec.t*rec.t*r.direction().length_squared();
        double cos_t=abs(dot(rec.normal,normalize(r.direction())));
        return dist_squared/(cos_t*area);
    }
    vec3 sample(const point3& origin,const double t)const override{
        point3 P=Q+random_double()*u+random_double()*v;
        return normalize(P-origin);
    }
  private:
    point3 Q;
    vec3 u,v;
    vec3 normal;
    shared_ptr<material> mat;
    bounding_box boundingbox;
    double area;
};

#endif