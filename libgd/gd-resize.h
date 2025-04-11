#ifndef GDRESIZE_H
#define GDRESIZE_H

#include "gd-image.h"

gdImagePtr gdImageScale(const gdImagePtr src,
                        const unsigned int new_width,
                        const unsigned int new_height);
gdInterpolationMethod gd_img_get_interpolation_method(gdImage *im);
int gd_img_set_interpolation_method(gdImage *im, gdInterpolationMethod id);

#endif // GDRESIZE_H


