#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "common.h"
#include "hittable.h"

class translate: public hittable{
  public:
    translate(const shared_ptr<hittable>& object, const vec3& offset): object(object), offset(offset){
        boundingbox=object->bbox().translate(offset);
    }

    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        ray offset_r(r.origin()-offset,r.direction(),r.time());
        if(!object->hit(offset_r,ray_t,rec))return 0;
        rec.p+=offset;
        return 1;
    }
    bounding_box bbox()const override{ return boundingbox;}
    double sample_pdf(const ray& r)const override{
        ray offset_r(r.origin()-offset,r.direction(),r.time());
        return object->sample_pdf(offset_r);
    }
    vec3 sample(const point3& origin, const double t)const override{
        return object->sample(origin-offset,t);
    }
  private:
    shared_ptr<hittable> object;
    vec3 offset;
    bounding_box boundingbox;
};

class rotate: public hittable{
  public: 
    rotate(const shared_ptr<hittable>& object, 
           double theta_x, double theta_y, double theta_z, const point3& center=point3(0,0,0)): 
        object(object), center(center){
        theta_x=deg_to_rad(theta_x),theta_y=deg_to_rad(theta_y),theta_z=deg_to_rad(theta_z);
        this->theta_x=theta_x, this->theta_y=theta_y, this->theta_z=theta_z;
        rotation_matrix=rotate_mat(theta_x,theta_y,theta_z);
        inv_rotation_matrix=inv(rotation_matrix);
        boundingbox=object->bbox().translate(-center).rotate(rotation_matrix).translate(center);
    }

    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        ray rotated_r(inv_rotation_matrix*(r.origin()-center)+center,inv_rotation_matrix*r.direction(),r.time());
        if(!object->hit(rotated_r,ray_t,rec))return 0;
        rec.p=rotation_matrix*(rec.p-center)+center;
        rec.normal=rotation_matrix*rec.normal;
        return 1;
    }
    bounding_box bbox()const override{ return boundingbox;}
    double sample_pdf(const ray& r)const override{
        ray rotated_r(inv_rotation_matrix*(r.origin()-center)+center,inv_rotation_matrix*r.direction(),r.time());
        return object->sample_pdf(rotated_r);
    }
    vec3 sample(const point3& origin, const double time)const override{
        point3 rotated_origin=inv_rotation_matrix*(origin-center)+center;
        return rotation_matrix*object->sample(rotated_origin,time);
    }

  private:
    shared_ptr<hittable> object;
    double theta_x,theta_y,theta_z;
    point3 center;
    mat3 rotation_matrix, inv_rotation_matrix;
    bounding_box boundingbox;
};

#endif