#ifndef MAT3_H
#define MAT3_H

#include "vec3.h"

class mat3{
  public:
    double a[3][3];
    mat3(){}
    mat3(double x){
        a[0][0]=a[1][1]=a[2][2]=x,a[0][1]=a[0][2]=a[1][0]=a[1][2]=a[2][0]=a[2][1]=0;
    }
    mat3(const double a00, const double a01, const double a02, 
         const double a10, const double a11, const double a12,
         const double a20, const double a21, const double a22){
        a[0][0]=a00,a[0][1]=a01,a[0][2]=a02,
        a[1][0]=a10,a[1][1]=a11,a[1][2]=a12,
        a[2][0]=a20,a[2][1]=a21,a[2][2]=a22;
    }
    mat3(const vec3& u, const vec3& v, const vec3& w){
        a[0][0]=u.x(),a[0][1]=v.x(),a[0][2]=w.x(),
        a[1][0]=u.y(),a[1][1]=v.y(),a[1][2]=w.y(),
        a[2][0]=u.z(),a[2][1]=v.z(),a[2][2]=w.z();
    }
    
    inline mat3 operator-()const{
        return mat3(-a[0][0],-a[0][1],-a[0][2],
                    -a[1][0],-a[1][1],-a[1][2],
                    -a[2][0],-a[2][1],-a[2][2]);
    }
    const double* operator[](int i)const{return a[i];}
    double* operator[](int i){return a[i];}
    inline mat3& operator+=(const mat3& b){
        a[0][0]+=b[0][0],a[0][1]+=b[0][1],a[0][2]+=b[0][2];
        a[1][0]+=b[1][0],a[1][1]+=b[1][1],a[1][2]+=b[1][2];
        a[2][0]+=b[2][0],a[2][1]+=b[2][1],a[2][2]+=b[2][2];
        return *this;
    }
    inline mat3& operator-=(const mat3& b){
        a[0][0]-=b[0][0],a[0][1]-=b[0][1],a[0][2]-=b[0][2];
        a[1][0]-=b[1][0],a[1][1]-=b[1][1],a[1][2]-=b[1][2];
        a[2][0]-=b[2][0],a[2][1]-=b[2][1],a[2][2]-=b[2][2];
        return *this;
    }
    inline mat3& operator*=(const double& t){
        a[0][0]*=t,a[0][1]*=t,a[0][2]*=t,
        a[1][0]*=t,a[1][1]*=t,a[1][2]*=t,
        a[2][0]*=t,a[2][1]*=t,a[2][2]*=t;
        return *this;
    }
    inline mat3& operator/=(const double& t){return *this*=1/t;}
    inline mat3& operator*=(const mat3& b){return *this=*this*b;}
    inline mat3 operator+(const mat3& b)  {
        return mat3(a[0][0]+b[0][0],a[0][1]+b[0][1],a[0][2]+b[0][2],
                    a[1][0]+b[1][0],a[1][1]+b[1][1],a[1][2]+b[1][2],
                    a[2][0]+b[2][0],a[2][1]+b[2][1],a[2][2]+b[2][2]);
    }
    inline mat3 operator-(const mat3& b){
        return mat3(a[0][0]-b[0][0],a[0][1]-b[0][1],a[0][2]-b[0][2],
                    a[1][0]-b[1][0],a[1][1]-b[1][1],a[1][2]-b[1][2],
                    a[2][0]-b[2][0],a[2][1]-b[2][1],a[2][2]-b[2][2]);
    }
    inline mat3 operator*(const mat3& b){
        mat3 tmp;
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                tmp[i][j]=a[i][0]*b[0][j]+a[i][1]*b[1][j]+a[i][2]*b[2][j];
        return tmp;
    }
    inline mat3 operator*(const double& x){
        return mat3(a[0][0]*x,a[0][1]*x,a[0][2]*x,
                    a[1][0]*x,a[1][1]*x,a[1][2]*x,
                    a[2][0]*x,a[2][1]*x,a[2][2]*x);
    }
    inline mat3 operator/(const double x){
        return mat3(a[0][0]/x,a[0][1]/x,a[0][2]/x,
                    a[1][0]/x,a[1][1]/x,a[1][2]/x,
                    a[2][0]/x,a[2][1]/x,a[2][2]/x);
    }
    inline double det()const{ 
        return a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])
              +a[1][0]*(a[2][1]*a[0][2]-a[2][2]*a[0][1])
              +a[2][0]*(a[0][1]*a[1][2]-a[0][2]*a[1][1]);
    }
    inline mat3 inv()const{
        return mat3(
            a[1][1]*a[2][2]-a[2][1]*a[1][2], a[2][1]*a[0][2]-a[0][1]*a[2][2],a[0][1]*a[1][2]-a[1][1]*a[0][2],
            a[1][2]*a[2][0]-a[2][2]*a[1][0],a[2][2]*a[0][0]-a[0][2]*a[2][0],a[0][2]*a[1][0]-a[1][2]*a[0][0],
            a[1][0]*a[2][1]-a[2][0]*a[1][1],a[2][0]*a[0][1]-a[0][0]*a[2][1],a[0][0]*a[1][1]-a[1][0]*a[0][1]
            )/det();
    }
    inline mat3 transpose()const{
        return mat3(a[0][0],a[1][0],a[2][0],a[0][1],a[1][1],a[2][1],a[0][2],a[1][2],a[2][2]);
    }
};
inline double det(const mat3& a){return a.det();}
inline mat3 inv(const mat3& a){return a.inv();}
inline mat3 transpose(const mat3& a){return a.transpose();}
inline vec3 operator*(const mat3& a, const vec3& v){
    return vec3(a[0][0]*v.e0+a[0][1]*v.e1+a[0][2]*v.e2,
                a[1][0]*v.e0+a[1][1]*v.e1+a[1][2]*v.e2,
                a[2][0]*v.e0+a[2][1]*v.e1+a[2][2]*v.e2);
}
inline std::ostream& operator<<(std::ostream& out, const mat3& a){
    for(int i=0;i<3;out<<'\n',i++)
        for(int j=0;j<3;j++)out<<a[i][j]<<' ';
    return out;
}


#endif