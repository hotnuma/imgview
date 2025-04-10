#ifndef GD_IMAGE_H
#define GD_IMAGE_H

#include <gdk-pixbuf/gdk-pixbuf.h>

/* Interpolation function ptr */
typedef double (* interpolation_method )(double, double);

#undef ARG_NOT_USED
#define ARG_NOT_USED(arg) (void) arg

/* resolution affects ttf font rendering, particularly hinting */
#define GD_RESOLUTION           96      /* pixels per inch */

/* If 'truecolor' is set true, the image is truecolor;
   pixels are represented by integers, which
   must be 32 bits wide or more.

   True colors are represented as follows:

   ARGB

   Where 'A' (alpha channel) occupies only the
   LOWER 7 BITS of the MSB. This very small
   loss of alpha channel resolution allows gd 2.x
   to keep backwards compatibility by allowing
   signed integers to be used to represent colors,
   and negative numbers to represent special cases,
   just as in gd 1.x. */

#define gdAlphaMax 255
#define gdAlphaOpaque 0
#define gdAlphaTransparent 255
#define gdRedMax 255
#define gdGreenMax 255
#define gdBlueMax 255


/**
 * Macro: gdImageSX
 *
 * Gets the width (in pixels) of an image.
 *
 * Parameters:
 *   im - The image.
 */
#define gdImageSX(im) ((im)->sx)

/**
 * Macro: gdImageSY
 *
 * Gets the height (in pixels) of an image.
 *
 * Parameters:
 *   im - The image.
 */
#define gdImageSY(im) ((im)->sy)

#define gd_set_alpha(r, g, b, a) (((a) << 24) + \
                                  ((r) << 16) + \
                                  ((g) <<  8) + \
                                   (b))

#define gd_get_red(c)   (((c) & 0xFF0000) >> 16)
#define gd_get_green(c) (((c) & 0x00FF00) >> 8)
#define gd_get_blue(c)   ((c) & 0x0000FF)
#define gd_get_alpha(c) (((c) & 0xFF000000) >> 24)

#if 0
#endif

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
    GD_DEFAULT          = 0,
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
    int trueColor;
    int **tpixels;
    int sx;
    int sy;
    gdInterpolationMethod interpolation_id;
    interpolation_method interpolation;

    /* 2.0.12: simple clipping rectangle. These values
      must be checked for safety when set; please use
      gdImageSetClip */
    int cx1;
    int cy1;
    int cx2;
    int cy2;
    int transparent;
}
gdImage;

typedef gdImage *gdImagePtr;

// libgd / pixbuf interface
GdkPixbufAnimation* gdk_pixbuf_non_anim_new (GdkPixbuf *pixbuf);
gdImage* pixbuf_to_gd(GdkPixbuf *pixbuf);
GdkPixbuf* gd_to_pixbuf(gdImage *src);


int gdImageBoundsSafe (gdImagePtr im, int x, int y);
/* 2.0.12: this now checks the clipping rectangle */
#define gdImageBoundsSafeMacro(im, x, y) (!((((y) < (im)->cy1) || ((y) > (im)->cy2)) || (((x) < (im)->cx1) || ((x) > (im)->cx2))))

/* Creates a truecolor image (millions of colors). */
gdImagePtr gd_img_new (int sx, int sy);

gdImagePtr gdImageClone (gdImagePtr src);
int gdImageSetInterpolationMethod(gdImagePtr im, gdInterpolationMethod id);
gdInterpolationMethod gdImageGetInterpolationMethod(gdImagePtr im);
void gd_img_free (gdImagePtr im);

gdImagePtr gdImageRotate90(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate180(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate270(gdImagePtr src, int ignoretransparent);

#endif // GD_IMAGE_H


