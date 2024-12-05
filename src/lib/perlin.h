#ifndef PERLIN_H
#define PERLIN_H

#include "common.h"
#include<algorithm>
#include<random>

class perlin_noise{
  public:
    perlin_noise(){
        for(int i=0;i<size;i++)rand_vec[i]=random_unit_vector();
        rand_perm(px),rand_perm(py),rand_perm(pz);
    }

    double noise(const point3& p)const{
        int i=floor(p.x()),j=floor(p.y()),k=floor(p.z());
        double u=p.x()-i,v=p.y()-j,w=p.z()-k;
        double c[2][2][2];
        for(int di=0;di<2;di++)
            for(int dj=0;dj<2;dj++)
                for(int dk=0;dk<2;dk++)
                    c[di][dj][dk]=dot(rand_vec[px[i+di&255]^py[j+dj&255]^pz[k+dk&255]],vec3(u-di,v-dj,w-dk));
        u=fade(u),v=fade(v),w=fade(w);
        return trilinear_interpolate(c,u,v,w);
    }
    double turb(const point3& p, int k)const{
        double res=0, w=1;
        while(k--)res+=noise(p*w)/w,w*=2;
        return abs(res);
    }
  private:
    static const int size=256;
    vec3 rand_vec[size];
    int px[size],py[size],pz[size];
    static void rand_perm(int *p){
        for(int i=0;i<size;i++)p[i]=i;
        std::shuffle(p,p+size,std::default_random_engine(rand()));
    }
    static double fade(double x){return x*x*x*((6*x-15)*x+10);}
};

#endif