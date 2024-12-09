#ifndef BVH_H
#define BVH_H

#include "common.h"
#include "bounding_box.h"
#include "hittable_list.h"

#include<algorithm>

class bvh_node: public hittable{
  public:
    bvh_node(hittable_list list): bvh_node(list.objects, 0, list.objects.size()){}
    bvh_node(std::vector<std::shared_ptr<hittable> >& objects, size_t start, size_t end){
        boundingbox=bounding_box::empty;
        for(size_t i=start;i<end;i++)
            boundingbox=bounding_box(boundingbox,objects[i]->bbox());
        int division_dim=argmax(boundingbox.x.size(),boundingbox.y.size(),boundingbox.z.size());
        auto box_cmp=(division_dim==0?box_cmp_x:(division_dim==1?box_cmp_y:box_cmp_z));
        size_t num_objects=end-start;
        if(num_objects==1)lson=objects[start],rson=nullptr;
        else if(num_objects==2)lson=objects[start],rson=objects[start+1];
        else{
            size_t mid=num_objects>>1;
            std::nth_element(objects.begin()+start,objects.begin()+start+mid,objects.begin()+end,box_cmp);
            lson=std::make_shared<bvh_node>(objects,start,start+mid);
            rson=std::make_shared<bvh_node>(objects,start+mid,end);
        }
    }
    bool hit(const ray& r, const interval& ray_t, hit_record& rec)const override{
        if(!boundingbox.hit(r,ray_t))return 0;
        bool hit_l=0,hit_r=0;
        if(lson!=nullptr)hit_l=lson->hit(r,ray_t,rec);
        if(rson!=nullptr)hit_r=rson->hit(r,interval(ray_t.min,hit_l?rec.t:ray_t.max),rec);
        return hit_l||hit_r;
    }
    bounding_box bbox()const override{ return boundingbox;}
    double sample_pdf(const ray& r)const override{
        if(!boundingbox.hit(r,interval(err,infty)))return 0;
        if(lson==nullptr&&rson==nullptr)return 1/(4*pi);
        if(rson==nullptr)return lson->sample_pdf(r);
        if(lson==nullptr)return rson->sample_pdf(r);
        return 0.5*(lson->sample_pdf(r)+rson->sample_pdf(r));
    }
    vec3 sample(const point3& origin, const double time)const override{
        if(lson==nullptr&&rson==nullptr)return random_unit_vector();
        if(rson==nullptr)return lson->sample(origin,time);
        if(lson==nullptr)return rson->sample(origin,time);
        return (rand()%2==0?lson:rson)->sample(origin,time);
    }
  private:
    std::shared_ptr<hittable> lson,rson;
    bounding_box boundingbox;
    static bool box_cmp_x(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
        return a->bbox().x.midpoint()<b->bbox().x.midpoint();
    }
    static bool box_cmp_y(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
        return a->bbox().y.midpoint()<b->bbox().y.midpoint();
    }
    static bool box_cmp_z(const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
        return a->bbox().z.midpoint()<b->bbox().z.midpoint();
    }
};

#endif