#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class vec3 {
  public:
    double e0,e1,e2;

    vec3(): e0(0),e1(0),e2(0) {}
    vec3(double e0, double e1, double e2): e0(e0), e1(e1), e2(e2){}

    double x()const{ return e0; }
    double y()const{ return e1; }
    double z()const{ return e2; }

    vec3 operator-()const{ return vec3(-e0,-e1,-e2); }
    double operator[](int i)const{ return i==0?e0:(i==1?e1:e2); }
    double& operator[](int i){ return i==0?e0:(i==1?e1:e2); }

    vec3& operator+=(const vec3 &v){ e0+=v.e0,e1+=v.e1,e2+=v.e2;return *this; }
    vec3& operator-=(const vec3 &v){ e0-=v.e0,e1-=v.e1,e2-=v.e2;return *this; }
    vec3& operator*=(const vec3 &v){ e0*=v.e0,e1*=v.e1,e2*=v.e2;return *this; }
    vec3& operator/=(const vec3 &v){ e0/=v.e0,e1/=v.e1,e2/=v.e2;return *this; }
    vec3& operator*=(double t){ e0*=t,e1*=t,e2*=t;return *this; }
    vec3& operator/=(double t){ e0/=t,e1/=t,e2/=t;return *this; }

    double length()const{ return std::sqrt(length_squared());}

    double length_squared()const{ return e0*e0+e1*e1+e2*e2; }
};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v){ return out<<v.e0<<' '<<v.e1<<' '<<v.e2;}
inline vec3 operator+(const vec3& u, const vec3& v){ return vec3(u.e0+v.e0,u.e1+v.e1,u.e2+v.e2);}
inline vec3 operator-(const vec3& u, const vec3& v){ return vec3(u.e0-v.e0,u.e1-v.e1,u.e2-v.e2);}
inline vec3 operator*(const vec3& u, const vec3& v){ return vec3(u.e0*v.e0,u.e1*v.e1,u.e2*v.e2);}
inline vec3 operator*(double t, const vec3& v){ return vec3(t*v.e0,t*v.e1,t*v.e2);}
inline vec3 operator*(const vec3& v, double t){ return t*v;}
inline vec3 operator/(const vec3& v, double t){ return (1/t)*v;}

inline double dot(const vec3& u, const vec3& v){ return u.e0*v.e0+u.e1*v.e1+u.e2*v.e2;}

inline vec3 cross(const vec3& u, const vec3& v){
    return vec3(u.e1*v.e2-u.e2*v.e1,
                u.e2*v.e0-u.e0*v.e2,
                u.e0*v.e1-u.e1*v.e0);
}
inline double length(const vec3& v){ return v.length();}
inline vec3 normalize(const vec3& v){ return v/length(v);}
inline vec3 random_unit_vector() {
    double z=random_double()*2-1,r=sqrt(1-z*z),theta=2*pi*random_double();
	return vec3(r*cos(theta),r*sin(theta),z);
}
inline vec3 random_unit_vector_in_hemisphere(const vec3& norm){
	vec3 v=random_unit_vector();
	return dot(v,norm)>0?v:-v;
}
inline vec3 reflect(const vec3& v,const vec3& n){ return v-2*dot(n,v)*n;}
inline vec3 refract(const vec3& v,const vec3& n,double eta_i_over_t){
	double cos_theta=std::min(-dot(v,n),1.);
	double sin_theta=sqrt(1.0-cos_theta*cos_theta);
	if(eta_i_over_t*sin_theta>=1.0)return reflect(v,n);
	double sin_theta_r=eta_i_over_t*sin_theta;
	double cos_theta_r=sqrt(1.0-sin_theta_r*sin_theta_r);
	
	double rs=square((eta_i_over_t*cos_theta-cos_theta_r)/(eta_i_over_t*cos_theta+cos_theta_r));
	double rp=square((eta_i_over_t*cos_theta_r-cos_theta)/(eta_i_over_t*cos_theta_r+cos_theta));
	double reflection_rate=(rs+rp)/2;//Fresnel law

	if(random_double()<reflection_rate)return reflect(v,n);

	vec3 rperp=eta_i_over_t*(v+cos_theta*n);
	vec3 rparal=-sqrt(abs(1-rperp.length_squared()))*n;
	return rperp+rparal;
}
inline vec3 random_lambertian_direction(){
	double phi=random_double()*2*pi;
	double r=random_double();
	return vec3(sqrt(r)*cos(phi),sqrt(r)*sin(phi),sqrt(1-r));
}

#endif