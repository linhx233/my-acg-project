#ifndef SCATTER_RECORD_H
#define SCATTER_RECORD_H

#include "common.h"
#include "pdf.h"

class scatter_record{
  public:
    color attenuation;
    bool using_importance_sampling;
    shared_ptr<pdf> sample_pdf;
    ray sample_ray;
};

#endif