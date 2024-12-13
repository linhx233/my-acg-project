#ifndef PDF_H
#define PDF_H

#include "common.h"
#include "hittable.h"

class pdf{
  public:
    virtual ~pdf(){}
    virtual double value(const vec3& v)const=0;
    virtual vec3 sample()const=0;
};

class uniform_pdf: public pdf{
  public:
    uniform_pdf(){}
    double value(const vec3& v)const override{return 1/(4*pi);}
    vec3 sample()const override{return random_unit_vector();}
};

class lambertian_pdf: public pdf{
  public:
    lambertian_pdf(const vec3& norm): B(norm){}
    double value(const vec3& v)const override{return std::max(1e-12,dot(B.w,normalize(v))/pi);}
    vec3 sample()const override{return B.to_standard(random_lambertian_direction());}
  private:
    orthonormal_basis B;
};

class directed_pdf: public pdf{
  public:
    directed_pdf(const hittable& object, const point3& origin):
        object(object), origin(origin){};
    double value(const vec3& v)const override{return object.sample_pdf(ray(origin,v));}
    vec3 sample()const override{return object.sample(origin);}
  private:
    const hittable& object;
    point3 origin;
};

class mixed_pdf: public pdf{
  public:
    mixed_pdf(const shared_ptr<pdf>& p0, const shared_ptr<pdf>& p1,const double lambda): 
        p0(p0), p1(p1), lambda(lambda){};
    double value(const vec3& v)const override{
        return lambda*p0->value(v)+(1-lambda)*p1->value(v);
    }
    vec3 sample()const override{
        return (random_double()<lambda?p0:p1)->sample();
    }
  private:
    shared_ptr<pdf> p0,p1;
    double lambda;
};

#endif