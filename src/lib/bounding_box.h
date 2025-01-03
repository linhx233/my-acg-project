#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "common.h"

class bounding_box{
  public:
    interval x,y,z;

    bounding_box(){}
    bounding_box(const interval& x,const interval& y, const interval &z): x(x), y(y), z(z){ pad();}
    bounding_box(const point3& a,const point3& b){
        x=a.x()<b.x()?interval(a.x(),b.x()):interval(b.x(),a.x());
        y=a.y()<b.y()?interval(a.y(),b.y()):interval(b.y(),a.y());
        z=a.z()<b.z()?interval(a.z(),b.z()):interval(b.z(),a.z());
        pad();
    }
    bounding_box(const bounding_box& a,const bounding_box& b){
        x=bounding_interval(a.x,b.x);
        y=bounding_interval(a.y,b.y);
        z=bounding_interval(a.z,b.z);
        pad();
    }

    double area(){return std::max(2*(x.size()*(y.size()+z.size())+y.size()*z.size()),0.);}

    bounding_box translate(const vec3& v){
        return bounding_box(x.translate(v.x()), y.translate(v.y()), z.translate(v.z()));
    }
    bounding_box rotate(const mat3& A){
        vec3 mn(infty,infty,infty),mx=-mn;
        for(int i=0;i<2;i++)
            for(int j=0;j<2;j++)
                for(int k=0;k<2;k++){
                    vec3 vertex(i?x.max:x.min,j?y.max:y.min,k?z.max:z.min);
                    vertex=A*vertex;
                    if(vertex.x()<mn.x())mn.e0=vertex.e0;
                    else if(vertex.x()>mx.x())mx.e0=vertex.e0;
                    if(vertex.y()<mn.y())mn.e1=vertex.e1;
                    else if(vertex.y()>mx.y())mx.e1=vertex.e1;
                    if(vertex.z()<mn.z())mn.e2=vertex.e2;
                    else if(vertex.z()>mx.z())mx.e2=vertex.e2;
                }
        return bounding_box(mn,mx);
    }

    bool hit(const ray& r, const interval& ray_t)const{
        interval rx=intersect_x(r),ry=intersect_y(r),rz=intersect_z(r);
        return std::max(std::max(ray_t.min,rx.min), std::max(ry.min,rz.min))
              <std::min(std::min(ray_t.max,rx.max), std::min(ry.max,rz.max));
    }
    static const bounding_box empty, universe;
  private:
    static const double eps;
    void pad(){
        if(x.size()<eps)x.expand(eps);
        if(y.size()<eps)y.expand(eps);
        if(z.size()<eps)z.expand(eps);
    }
    inline interval intersect_x(const ray& r)const{
        double x0=r.origin().x(),dx=r.direction().x();
        if(abs(dx)>err){
            double t0=(x.min-x0)/dx,t1=(x.max-x0)/dx;
            return t0<t1?interval(t0,t1):interval(t1,t0);
        }
        return x.contains(x0)?interval::universe:interval::empty;
    }
    inline interval intersect_y(const ray& r)const{
        double y0=r.origin().y(),dy=r.direction().y();
        if(abs(dy)>err){
            double t0=(y.min-y0)/dy,t1=(y.max-y0)/dy;
            return t0<t1?interval(t0,t1):interval(t1,t0);
        }
        return y.contains(y0)?interval::universe:interval::empty;
    }
    inline interval intersect_z(const ray& r)const{
        double z0=r.origin().z(),dz=r.direction().z();
        if(abs(dz)>err){
            double t0=(z.min-z0)/dz,t1=(z.max-z0)/dz;
            return t0<t1?interval(t0,t1):interval(t1,t0);
        }
        return z.contains(z0)?interval::universe:interval::empty;
    }
};

const bounding_box bounding_box::empty=bounding_box(interval::empty,interval::empty,interval::empty);
const bounding_box bounding_box::universe=bounding_box(interval::universe,interval::universe,interval::universe);
const double bounding_box::eps=1e-6;

#endif