#ifndef VEC2_H
#define VEC2_H

#include <cmath>

class vec2{
  public:
    double u,v;
    vec2(): u(0), v(0){}
    vec2(double u, double v): u(u), v(v){}
	vec2(const aiVector3D& b): u(b.x), v(b.y){}
    
	vec2 operator-()const{ return vec2(-u,-v); }
	vec2& operator+=(const vec2 &b){ u+=b.u,v+=b.v;return *this; }
	vec2& operator-=(const vec2 &b){ u-=b.u,v-=b.v;return *this; }
	vec2& operator*=(const vec2 &b){ u*=b.u,v*=b.v;return *this; }
	vec2& operator/=(const vec2 &b){ u/=b.u,v/=b.v;return *this; }
	vec2& operator*=(double t){ u*=t,v*=t;return *this; }
	vec2& operator/=(double t){ u/=t,v/=t;return *this; }

	double length()const{ return std::sqrt(length_squared());}
	double length_squared()const{ return u*u+v*v; }
};

inline vec2 operator+(const vec2& u, const vec2& v){ return vec2(u.u+v.u,u.v+v.v);}
inline vec2 operator-(const vec2& u, const vec2& v){ return vec2(u.u-v.u,u.v-v.v);}
inline vec2 operator*(const vec2& u, const vec2& v){ return vec2(u.u*v.u,u.v*v.v);}
inline vec2 operator*(double t, const vec2& v){ return vec2(t*v.u,t*v.v);}
inline vec2 operator*(const vec2& v, double t){ return t*v;}
inline vec2 operator/(const vec2& v, double t){ return (1/t)*v;}

inline double length(const vec2& v){ return v.length();}
inline vec2 normalize(const vec2& v){ return v/length(v);}

using point2 = vec2;

#endif