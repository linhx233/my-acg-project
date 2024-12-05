#ifndef MAKE_BOX_H
#define MAKE_BOX_H

#include "common.h"
#include "quad.h"
#include "hittable_list.h"

inline shared_ptr<hittable_list> make_box(const point3& a,const point3& b, shared_ptr<material> mat){
    shared_ptr<hittable_list> walls=make_shared<hittable_list>();
    point3 mn(std::min(a.x(),b.x()),std::min(a.y(),b.y()),std::min(a.z(),b.z())),
           mx(std::max(a.x(),b.x()),std::max(a.y(),b.y()),std::max(a.z(),b.z()));
    vec3 dx(mx.x()-mn.x(),0,0),dy(0,mx.y()-mn.y(),0),dz(0,0,mx.z()-mn.z());
    walls->add(make_shared<quad>(point3(mn.x(), mn.y(), mx.z()),  dx,  dy, mat));
    walls->add(make_shared<quad>(point3(mx.x(), mn.y(), mx.z()), -dz,  dy, mat));
    walls->add(make_shared<quad>(point3(mx.x(), mn.y(), mn.z()), -dx,  dy, mat));
    walls->add(make_shared<quad>(point3(mn.x(), mn.y(), mn.z()),  dz,  dy, mat));
    walls->add(make_shared<quad>(point3(mn.x(), mx.y(), mx.z()),  dx, -dz, mat));
    walls->add(make_shared<quad>(point3(mn.x(), mn.y(), mn.z()),  dx,  dz, mat));
    return walls;
}

#endif