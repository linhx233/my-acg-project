#ifndef SCATTER_RECORD_H
#define SCATTER_RECORD_H

#include "common.h"
#include "pdf.h"
#include<functional>
using std::function;

class scatter_record{
  public:
	function<const color(const vec3&)> attenuation;
	bool using_importance_sampling;
	shared_ptr<pdf> sample_pdf;
	ray sample_ray;
	bool path_unchanged=0;
};

#endif