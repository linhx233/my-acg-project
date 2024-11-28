#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include <vector>
#include <memory>

class hittable_list: public hittable{
  public:
    std::vector<std::shared_ptr<hittable> > objects;
    hittable_list(){}
    hittable_list(std::shared_ptr<hittable> object){ clear(),add(object);}
    inline void add(std::shared_ptr<hittable> object){ 
        objects.push_back(object);
        boundingbox=bounding_box(boundingbox,object->bbox());
    }
    inline void clear(){ objects.clear();}
    
    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const{
        bool flag=0;
        hit_record tmp;
        double ctmin=ray_t.max;
        for(auto obj:objects)
            if(obj->hit(r,ray_t,tmp))
                if(tmp.t<ctmin)ctmin=tmp.t,rec=tmp,flag=1;
        return flag;
    }

    bounding_box bbox()const override{ return boundingbox;}
  private:
    bounding_box boundingbox;
};

#endif