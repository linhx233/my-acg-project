#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "common.h"
#include "hittable.h"

class triangle: public hittable{
  public:
    triangle(const point3& A, const point3& B, const point3& C, const shared_ptr<material>& mat) 
        :A(A), B(B), C(C), mat(mat){
        vec3 v=cross(B-A,C-A);
        area=length(v),normal=normalize(v);
        boundingbox=bounding_box(bounding_box(A,B),bounding_box(C,C));
    }

    const void* get_pointer()const override{return this;}
    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        mat3 M(-r.direction(),B-A,C-A);
        vec3 b=r.origin()-A;
        if(abs(det(M))<err)return 0;
        vec3 v=inv(M)*b;
        if(v[1]<0||v[2]<0||v[1]+v[2]>1||!ray_t.contains(v[0]))return 0;
        rec.t=v[0];
        rec.p=r.at(rec.t);
        rec.set_normal(r,normal);
        rec.mat=mat;
        rec.tex_coord=point2(v[1],v[2]);
        rec.obj=get_pointer();
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
        double u=random_double(),v=random_double();
        if(u+v>1)u=1-u,v=1-v;  
        point3 P=(1-u-v)*A+u*B+v*C;
        return normalize(P-origin);
    }
  private:
    point3 A,B,C;
    vec3 normal;
    shared_ptr<material> mat;
    bounding_box boundingbox;
    double area;
};

#endif