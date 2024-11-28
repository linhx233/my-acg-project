#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"
#include "hit_record.h"
#include "bounding_box.h"

class hittable{
  public:
    virtual ~hittable()=default;
    virtual bool hit(const ray& r, const interval& ray_t, hit_record& rec)const=0;
	virtual bounding_box bbox() const = 0;
};

#endif