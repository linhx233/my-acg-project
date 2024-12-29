#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "pdf.h"
#include "scatter_record.h"
#include "vec3.h"

class material{
  public:
	virtual ~material()=default;
	virtual bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const{return 0;}
	virtual vec3 bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const{return 0;}
	virtual color emit(const ray& ray_in, const hit_record& rec)const{return color(0,0,0);}
};

class lambertian: public material{
  public:
	lambertian(color albedo): tex(make_shared<solid_color>(solid_color(albedo))){}
	lambertian(const shared_ptr<texture>& tex): tex(tex){}

	bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
		vec3 scattered_direction=random_unit_vector()+rec.normal;
		while(scattered_direction.length_squared()<1e-12)
			scattered_direction=random_unit_vector()+rec.normal;
		scatter.attenuation=[=](const vec3& v)->const color{return tex->value(rec.tex_coord,rec.p);};
		scatter.using_importance_sampling=true;
		scatter.sample_pdf=make_shared<lambertian_pdf>(rec.normal);
		return 1;
	}
	vec3 bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const override{
		return std::max(0.,dot(rec.normal,normalize(ray_out.direction())/pi));
	}
  private:
	shared_ptr<texture> tex;
};

class metal: public material{
  public:
	metal(color albedo, double fuzz): tex(make_shared<solid_color>(albedo)), fuzz(std::min(fuzz,1.)){}
	metal(const shared_ptr<texture>& tex, double fuzz): tex(tex), fuzz(std::min(fuzz,1.)){}
	bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
		vec3 new_direction=normalize(reflect(ray_in.direction(),rec.normal))+random_unit_vector()*fuzz;
		scatter.attenuation=[=](const vec3& v)->const color{return tex->value(rec.tex_coord,rec.p);};
		scatter.using_importance_sampling=false;
		scatter.sample_ray=ray(rec.p,new_direction,ray_in.time());
		return dot(new_direction,rec.normal)>0;
	}
  private:
	shared_ptr<texture> tex;
	double fuzz;
};

class dielectric: public material{
  public:
	dielectric(double refraction_rate, color albedo=color(1.0,1.0,1.0)):
		tex(make_shared<solid_color>(albedo)),refraction_rate(refraction_rate){}
	bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
		vec3 in_direction=normalize(ray_in.direction());
		double eta=rec.outer_face?1/refraction_rate:refraction_rate;
		scatter.attenuation=[=](const vec3& v)->const color{return tex->value(rec.tex_coord,rec.p);};
		scatter.using_importance_sampling=false;
		scatter.sample_ray=ray(rec.p,refract(in_direction,rec.normal,eta),ray_in.time());
		return 1;
	}
  private:
	shared_ptr<texture> tex;
	double refraction_rate;
};

class diffuse_light: public material {
  public:
	diffuse_light(shared_ptr<texture> tex, 
				  double attenuation_const=0, 
				  double attenuation_linear=0, 
				  double attenuation_quadratic=1)
		:tex(tex),
		 attenuation_const(attenuation_const),
		 attenuation_linear(attenuation_linear),
		 attenuation_quadratic(attenuation_quadratic){}
	diffuse_light(const color& emit, 
				  double attenuation_const=0, 
				  double attenuation_linear=0, 
				  double attenuation_quadratic=1)
		:tex(make_shared<solid_color>(emit)),
		 attenuation_const(attenuation_const),
		 attenuation_linear(attenuation_linear),
		 attenuation_quadratic(attenuation_quadratic){}

	color emit(const ray& ray_in, const hit_record& rec)const override{ 
		double invd=1/length(rec.p-ray_in.origin());
		double atten=1/((attenuation_const*invd+attenuation_linear)*invd+attenuation_quadratic);
		return (tex->value(rec.tex_coord,rec.p))*atten;
	}
  private:
	shared_ptr<texture> tex;
	double attenuation_const,attenuation_linear,attenuation_quadratic;
};

class transparent: public material{
  public:
	transparent(const shared_ptr<material>& mat, double alpha=1): alpha(alpha), mat(mat){}

	bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter)const override{
		if(random_double()>=alpha){
			scatter.attenuation=[=](const vec3& v)->const color{return color(1,1,1);};
			scatter.using_importance_sampling=false;
			scatter.sample_ray=ray(rec.p,ray_in.direction(),ray_in.time());
			return true;
		}
		bool scattered=mat->scatter(ray_in,rec,scatter);
		return scattered;
	}
	vec3 bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out)const override{
		return alpha*mat->bsdf(ray_in,rec,ray_out);
	}
	color emit(const ray& ray_in, const hit_record& rec)const override{return mat->emit(ray_in,rec);}
  private:
	double alpha;
	shared_ptr<material> mat;
};

class PrincipledBSDF : public material {
  private:
    std::shared_ptr<texture> base_color;
    std::shared_ptr<texture> normal_map;
	std::shared_ptr<texture> emitted;
	double emitted_intensity;
    double roughness;
    double metalness;
    double opacity;
    double ior;
	double transmission;
    vec3 f0;  // Fresnel
	double alpha;

    void calculate_f0() {
        vec3 dielectric_f0 = vec3(square((ior - 1.0) / (ior + 1.0)));
        vec3 metalness_f0 = base_color ? base_color->value(point2(0, 0), point3(0, 0, 0)) : vec3(0.0);
        f0 = metalness * metalness_f0 + (1.0 - metalness) * dielectric_f0;
    }

	double pow5(double x)const{double y=x*x;return y*y*x;}

    vec3 schlick_fresnel(double cos_theta) const {
        return f0 + (vec3(1.0) - f0) * pow5(1.0 - cos_theta);
    }

    double distribution_ggx(double hon) const {
		if(hon <= 0)return 0;
        double alpha2 = alpha * alpha;
        double denom = hon * hon * (alpha2 - 1.0) + 1.0;
        return alpha2 / (pi * denom * denom);
    }

	vec3 ggx_sample(const vec3& n) const {
		double u1 = random_double();
		double u2 = random_double();

		double theta = atan(alpha * sqrt(u1 / (1.0 - u1)));
		double phi = 2.0 * pi * u2;

		double sin_theta = sin(theta);
		double cos_theta = cos(theta);

		return orthonormal_basis(n).to_standard(vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta));
	}

    double geometry_smith(double von) const {
        double tan2_theta = 1.0 / square(von + 1e-50) - 1.0;
        return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tan2_theta));
    }

  public:
    PrincipledBSDF(const std::shared_ptr<texture>& base_color,
                   const std::shared_ptr<texture>& normal_map,
				   const std::shared_ptr<texture>& emitted,
				   double emitted_intensity,
                   double roughness, 
                   double metalness, 
                   double opacity, 
                   double ior,
				   double transmission)
        : base_color(base_color), 
          normal_map(normal_map), 
		  emitted(emitted),
		  emitted_intensity(emitted_intensity),
          roughness(std::max(roughness,0.001)), 
          metalness(metalness), 
          opacity(opacity), 
          ior(ior),
		  transmission(transmission) {
        calculate_f0();
		alpha=roughness*roughness;
    }

    bool scatter(const ray& ray_in, const hit_record& rec, scatter_record& scatter) const override {
        vec3 n = rec.normal; // Surface normal
        vec3 v = -normalize(ray_in.direction()); // View vector

        // Handle opacity
        if (random_double() >= opacity) {
            scatter.using_importance_sampling = false;
			scatter.path_unchanged = true;
            scatter.sample_ray = ray(rec.p, ray_in.direction(), ray_in.time());
            scatter.attenuation = [](const vec3&) { return vec3(1.0); }; // Pass through
            return true;
        }

		const double max_transmission_ratio = 0.8;

		double ratio_transmission = transmission * (1 - metalness) * max_transmission_ratio;
		if(random_double() < ratio_transmission){
			vec3 h = ggx_sample(n);
			int num_attempt = 1;//Try to sample a good microfacet
			while(dot(v,h) <= 0 && num_attempt < 10)h = ggx_sample(n), num_attempt++;
			double cos_theta_v = dot(v,h);
			if(cos_theta_v <= 0)return false;

			double sin_theta_v = sqrt(1 - cos_theta_v * cos_theta_v);
			double eta = rec.outer_face ? 1 / ior : ior;
			double sin_theta_l = eta * sin_theta_v;
			if(sin_theta_l > 1) return false;//Total incidence reflection

			double cos_theta_l = sqrt(1 - square(sin_theta_l));
			vec3 rperp = eta * (-v + cos_theta_v * h);
			vec3 rparal = -sqrt(abs(1 - rperp.length_squared())) * h;
			vec3 l = rperp + rparal;//Compute refracted direction
			// if(dot(l,h)>0|| length(cross(l,-h) - eta * cross(v,h))>1e-5){
			// 	std::clog<<"Wrong refraction!"<<std::endl;
			// 	exit(0);
			// }

			double rs=square((eta * cos_theta_v - cos_theta_l) / (eta * cos_theta_v + cos_theta_l));
			double rp=square((eta * cos_theta_l - cos_theta_v) / (eta * cos_theta_l + cos_theta_v));
			double f = (rs + rp) / 2;//Use accurate Fresnel here

        	double g = geometry_smith(dot(v,n)) * geometry_smith(dot(l,n));

			scatter.using_importance_sampling = false;
			scatter.sample_ray = ray(rec.p, l, ray_in.time());
			scatter.attenuation = [=](const vec3& p){
				return vec3(1,1,1)* (1-f) * g * cos_theta_v / 
					   ((abs(dot(v,n)) * dot(h,n) + 1e-50) * num_attempt * max_transmission_ratio);
			};
			return true;
		}

        //If roughness not very low, use importance sampling
		if(roughness > 0.1){
			scatter.using_importance_sampling = true;
			auto pdf_diffuse   = make_shared<lambertian_pdf>(n);
			auto pdf_specular  = make_shared<ggx_reflect_pdf>(n, v, roughness);
			scatter.sample_pdf = make_shared<mixed_pdf>(pdf_diffuse, pdf_specular, 
														(0.6 + 0.2 * roughness) * (1 - metalness));
			scatter.attenuation = [=](const vec3& v){return color(1,1,1) / (1 - ratio_transmission);};
		}
		else{//Otherwise there would be a sharp highlight, process diff and spec separately
			double ratio = (0.6 + 0.2 * roughness) * (1 - metalness);
			if(random_double() < ratio){
				scatter.using_importance_sampling = true;
				scatter.sample_pdf    = make_shared<lambertian_pdf>(n);
				scatter.attenuation = [=](const vec3& l){
					vec3 ll = normalize(l);
					vec3 h = normalize(v + l); // Half vector
					double von=dot(v,n), lon=dot(l,n);
					if( von<=0 || lon<=0)return vec3(1,1,1);
					double voh=dot(v,h),hon=dot(h,n);

        			// Specular term
        			vec3 f = schlick_fresnel(voh);
        			double g = geometry_smith(von) * geometry_smith(lon);
        			double d = distribution_ggx(hon);
        			vec3 specular = f * g * d / (4.0 * abs(von) + 1e-50);

        			// Diffuse term
					double fd90 = 0.5 + 2 * roughness * square(voh);
					double fdv = 1 + (fd90 - 1) * pow5(1 - von);
					double fdl = 1 + (fd90 - 1) * pow5(1 - lon);
        			vec3 kd = (1 - metalness) * fdv * fdl;
					color albedo = base_color ? base_color->value(rec.tex_coord, rec.p) : color(0,0,0);
        			vec3 diffuse = kd * albedo * lon / pi;

        			// Combine diffuse and specular
        			return vec3(1.) / ((1 - ratio_transmission) * ratio * (vec3(1.) + make_safe(specular / diffuse)));
				};
			}
			else{// Direct sample specular to avoid singular D value;
				scatter.using_importance_sampling=false;
				auto ggx=make_shared<ggx_reflect_pdf>(n, v, roughness);
				vec3 dir=ggx->sample();
				scatter.sample_ray=ray(rec.p,dir,ray_in.time());
				scatter.attenuation = [=](const vec3 &l){
					vec3 h = normalize(v + l);
					double voh=dot(v,h), von=dot(v,n), lon=dot(l,n), hon=dot(h,n);
					vec3 f = schlick_fresnel(voh);
        			double g = geometry_smith(von) * geometry_smith(lon);
        			vec3 specular = f * g * abs(voh) / (abs(von) * hon+ 1e-50);
					return (max(make_safe(specular),0)) / ((1 - ratio_transmission) * (1 - ratio));
				};
			}
		}
        return true;
    }

    vec3 bsdf(const ray& ray_in, const hit_record& rec, const ray& ray_out) const override {
        vec3 n = normalize(rec.normal); // Surface normal
        vec3 v = -normalize(ray_in.direction()); // View vector
        vec3 l = normalize(ray_out.direction()); // Light vector
        vec3 h = normalize(v + l); // Half vector
		double von=dot(v,n), lon=dot(l,n);
		if(von<=0)return 0;
		if(lon<=0)return 0;//Transmissive contribution won't be importance sampled
		double voh=dot(v,h),hon=dot(h,n);

        // Specular term
        vec3 f = schlick_fresnel(voh);
        double g = geometry_smith(von) * geometry_smith(lon);
        double d = distribution_ggx(hon);
        vec3 specular = f * g * d / (4.0 * abs(von) + 1e-50);

        // Diffuse term
		double fd90 = 0.5 + 2 * roughness * square(voh);
		double fdv = 1 + (fd90 - 1) * pow5(1 - von);
		double fdl = 1 + (fd90 - 1) * pow5(1 - lon);
        vec3 kd = (1 - metalness) * fdv * fdl;
		color albedo = base_color ? base_color->value(rec.tex_coord, rec.p) : color(0,0,0);
        vec3 diffuse = kd * albedo * lon / pi;

        // Combine diffuse and specular
        return max(make_safe(diffuse),0) + max(make_safe(specular),0);
    }

    color emit(const ray& ray_in, const hit_record& rec) const override {
        return emitted_intensity*(emitted ? emitted->value(rec.tex_coord, rec.p): color(0,0,0));
    }
};




#endif