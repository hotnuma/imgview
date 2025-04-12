#ifndef GD_IMAGE_H
#define GD_IMAGE_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <inttypes.h>
#include <stdbool.h>

typedef double (*interpolation_method) (double, double);

#define gd_img_sx(im) ((im)->sx)
#define gd_img_sy(im) ((im)->sy)
#define gd_set_alpha(r, g, b, a) (((r) << 24) + \
                                  ((g) << 16) + \
                                  ((b) <<  8) + \
                                   (a))
#define gd_get_red(c) (((c) & 0xFF000000) >> 24)
#define gd_get_green(c)   (((c) & 0xFF0000) >> 16)
#define gd_get_blue(c) (((c) & 0x00FF00) >> 8)
#define gd_get_alpha(c)   ((c) & 0x0000FF)

typedef enum
{
    HORIZONTAL,
    VERTICAL,

} gdAxis;

/**
 *
 * Constants: gdInterpolationMethod
 *
 *  GD_BELL				 - Bell
 *  GD_BESSEL			 - Bessel
 *  GD_BILINEAR_FIXED 	 - fixed point bilinear
 *  GD_BICUBIC 			 - Bicubic
 *  GD_BICUBIC_FIXED 	 - fixed point bicubic integer
 *  GD_BLACKMAN			 - Blackman
 *  GD_BOX				 - Box
 *  GD_BSPLINE			 - BSpline
 *  GD_CATMULLROM		 - Catmullrom
 *  GD_GAUSSIAN			 - Gaussian
 *  GD_GENERALIZED_CUBIC - Generalized cubic
 *  GD_HERMITE			 - Hermite
 *  GD_HAMMING			 - Hamming
 *  GD_HANNING			 - Hannig
 *  GD_MITCHELL			 - Mitchell
 *  GD_NEAREST_NEIGHBOUR - Nearest neighbour interpolation
 *  GD_POWER			 - Power
 *  GD_QUADRATIC		 - Quadratic
 *  GD_SINC				 - Sinc
 *  GD_TRIANGLE       - Triangle
 *  GD_WEIGHTED4		 - 4 pixels weighted bilinear interpolation
 *  GD_LINEAR            - bilinear interpolation
 *
 */
typedef enum
{
    GD_DEFAULT = 0,
    GD_BELL,
    GD_BESSEL,
    GD_BILINEAR_FIXED,
    GD_BICUBIC,
    GD_BICUBIC_FIXED,
    GD_BLACKMAN,
    GD_BOX,
    GD_BSPLINE,
    GD_CATMULLROM,
    GD_GAUSSIAN,
    GD_GENERALIZED_CUBIC,
    GD_HERMITE,
    GD_HAMMING,
    GD_HANNING,
    GD_MITCHELL,
    GD_NEAREST_NEIGHBOUR,
    GD_POWER,
    GD_QUADRATIC,
    GD_SINC,
    GD_TRIANGLE,
    GD_WEIGHTED4,
    GD_LINEAR,
    GD_LANCZOS3,
    GD_LANCZOS8,
    GD_BLACKMAN_BESSEL,
    GD_BLACKMAN_SINC,
    GD_QUADRATIC_BSPLINE,
    GD_CUBIC_SPLINE,
    GD_COSINE,
    GD_WELSH,
    GD_METHOD_COUNT = 30

} gdInterpolationMethod;

typedef struct gdImageStruct
{
    uint32_t **tpixels;
    bool has_alpha;
    int sx;
    int sy;
    gdInterpolationMethod interpolation_id;
    interpolation_method interpolation;

    int cx1;
    int cy1;
    int cx2;
    int cy2;
    int transparent;

} gdImage;

// creation / destruction -----------------------------------------------------

gdImage* gd_img_new(int sx, int sy);
GdkPixbufAnimation* gdk_pixbuf_non_anim_new (GdkPixbuf *pixbuf);
gdImage* gd_img_new_from_pixbuf(GdkPixbuf *pixbuf);
void gd_img_free(gdImage* im);

GdkPixbuf* gd_to_pixbuf(gdImage *src);
gdImage* gd_img_copy(const gdImage *src);

#define gd_img_bounds_safe_macro(im, x, y) \
    (!((((y) < (im)->cy1) || ((y) > (im)->cy2)) \
    || (((x) < (im)->cx1) || ((x) > (im)->cx2))))

#endif // GD_IMAGE_H


