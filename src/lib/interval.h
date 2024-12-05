#ifndef INTERVAL_H
#define INTERVAL_H

class interval{
  public:
	double min,max;

	interval(): min(+infty),max(-infty){}
	interval(double min, double max): min(min), max(max){}

	inline double size()const{ return max-min;}
	inline bool contains(double x)const{ return min<=x&&x<=max;}
	inline bool surrounds(double x)const{ return min<x&&x<max;}
	inline double clamp(double x)const{ return x<min?min:(x>max?max:x);}
	inline double midpoint()const{ return (min+max)*0.5;}
	inline void expand(double x){min-=x/2,max+=x/2;}
	inline interval translate(const double t)const{return interval(min+t,max+t);}

	static const interval empty, universe, ratio;
};

const interval interval::empty=interval(+infty, -infty);
const interval interval::universe=interval(-infty, +infty);
const interval interval::ratio=interval(0,1);

inline interval bounding_interval(const interval& a,const interval& b){
	return interval(std::min(a.min,b.min),std::max(a.max,b.max));
}
inline interval intersect(const interval &a,const interval &b){
	double l=std::max(a.min,b.min),r=std::min(a.max,b.max);
	return l>r?interval::empty:interval(l,r);
}

#endif