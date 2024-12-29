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
	directed_pdf(const shared_ptr<hittable> object, const point3& origin):
		object(object), origin(origin){};
	double value(const vec3& v)const override{return object->sample_pdf(ray(origin,v));}
	vec3 sample()const override{return object->sample(origin);}
  private:
	const shared_ptr<hittable> object;
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

class ggx_reflect_pdf : public pdf {
  private:
	orthonormal_basis onb; // Surface normal
	vec3 v; // View vector
	double alpha; // Roughness squared

	double value0(const vec3& l) const{
		vec3 ll=normalize(l);
		vec3 h = normalize(v+ll); // Half vector
		double cos_theta_h = dot(h, onb.w)+1e-50;
		if(cos_theta_h<=0) h=-h, cos_theta_h=-cos_theta_h;

		double alpha2 = alpha * alpha;
		double denom = (cos_theta_h * cos_theta_h) * (alpha2 - 1.0) + 1.0;
		double D = alpha2 / (pi * denom * denom);

		// PDF value for sampling the direction
		return std::max(0., D * cos_theta_h / (4.0 * abs(dot(v, h)) + 1e-50));
	}  

  public:
	ggx_reflect_pdf(const vec3& normal, const vec3& view_dir, double roughness)
		: onb(normal), v(normalize(view_dir)), alpha(roughness * roughness) {}

	double value(const vec3& l) const override{
		// return std::max(1e-12,dot(onb.w,normalize(l))/pi);
		return value0(l)+value0(-l);
	}

	vec3 sample() const override {
		// return onb.to_standard(random_lambertian_direction());
		// Importance sample GGX distribution for the half-vector
		double u1 = random_double();
		double u2 = random_double();

		double theta = atan(alpha * sqrt(u1 / (1.0 - u1)));
		double phi = 2.0 * pi * u2;

		double sin_theta = sin(theta);
		double cos_theta = cos(theta);

		vec3 h=onb.to_standard(vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta));

		// Transform half-vector `h` to scattered direction `l`
		vec3 l = reflect(v, h);
		if (dot(l, onb.w) < 0.0) l = -l;

		return l;
	}
};


#endif