#ifndef HIT_RECORD_H
#define HIT_RECORD_H

#include "common.h"

class material;
class hittable;

class hit_record{
  public:
	const void* obj;
	point3 p;
	vec3 normal;
	bool outer_face;
	std::shared_ptr<material> mat;
	double t;
	point2 tex_coord;
	inline void set_normal(const ray& r,const vec3& outward_normal){
		normal= dot(r.direction(),outward_normal)<0 ? outer_face=1,outward_normal : (outer_face=0,-outward_normal);
	}
};

#endif