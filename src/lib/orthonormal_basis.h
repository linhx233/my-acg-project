#ifndef ORTHONORMAL_BASIS_H
#define ORTHONORMAL_BASIS_H

#include "common.h"

class orthonormal_basis{
  public:
    vec3 u,v,w;
    orthonormal_basis(const vec3& w): w(normalize(w)){
		vec3 a=random_unit_vector();
		while(abs(dot(a,w))<err)a=random_unit_vector();
		u=normalize(cross(a,w));
		v=cross(w,u);
    }
	orthonormal_basis(const vec3& u, const vec3& v, const vec3&w): u(u), v(v), w(w){}

	vec3 to_standard(const vec3& a)const{return a.x()*u+a.y()*v+a.z()*w;}
	vec3 to_this(const vec3& a)const{return transpose(mat3(u,v,w))*a;}
};

#endif