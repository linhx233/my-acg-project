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
  private:
    point3 Q;
    vec3 u,v;
    vec3 normal;
    shared_ptr<material> mat;
    bounding_box boundingbox;
};

#endif