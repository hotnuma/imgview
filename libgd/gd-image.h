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

/**
 * Group: Color Composition
 *
 * Macro: gdTrueColorAlpha
 *
 * Compose a truecolor value from its components
 *
 * Parameters:
 *   r - The red channel (0-255)
 *   g - The green channel (0-255)
 *   b - The blue channel (0-255)
 *   a - The alpha channel (0-255, where 255 is fully transparent, and 0 is
 *       completely opaque).
 *
 * See also:
 *   - <gdTrueColorGetAlpha>
 *   - <gdTrueColorGetRed>
 *   - <gdTrueColorGetGreen>
 *   - <gdTrueColorGetBlue>
 *   - <gdImageColorExactAlpha>
 */
#define gdTrueColorAlpha(r, g, b, a) (((a) << 24) + \
                      ((r) << 16) + \
                      ((g) << 8) +  \
                      (b))

/**
 * Macro: gdTrueColorGetAlpha
 *
 * Gets the alpha channel value
 *
 * Parameters:
 *   c - The color
 *
 * See also:
 *   - <gdTrueColorAlpha>
 */
#define gdTrueColorGetAlpha(c) (((c) & 0xFF000000) >> 24)

/**
 * Macro: gdTrueColorGetRed
 *
 * Gets the red channel value
 *
 * Parameters:
 *   c - The color
 *
 * See also:
 *   - <gdTrueColorAlpha>
 */
#define gdTrueColorGetRed(c) (((c) & 0xFF0000) >> 16)

/**
 * Macro: gdTrueColorGetGreen
 *
 * Gets the green channel value
 *
 * Parameters:
 *   c - The color
 *
 * See also:
 *   - <gdTrueColorAlpha>
 */
#define gdTrueColorGetGreen(c) (((c) & 0x00FF00) >> 8)

/**
 * Macro: gdTrueColorGetBlue
 *
 * Gets the blue channel value
 *
 * Parameters:
 *   c - The color
 *
 * See also:
 *   - <gdTrueColorAlpha>
 */
#define gdTrueColorGetBlue(c) ((c) & 0x0000FF)

typedef enum {
    HORIZONTAL,
    VERTICAL,
} gdAxis;

/**
 * Group: Types
 *
 * typedef: gdPointF
 *  Defines a point in a 2D coordinate system using floating point
 *  values.
 * x - Floating point position (increase from left to right)
 * y - Floating point Row position (increase from top to bottom)
 *
 * typedef: gdPointFPtr
 *  Pointer to a <gdPointF>
 *
 * See also:
 *  <gdImageCreate>, <gdImageCreateTrueColor>,
 **/
typedef struct
{
    double x, y;
}
gdPointF, *gdPointFPtr;



/*
  Group: Types

  typedef: gdPoint

  typedef: gdPointPtr

  Represents a point in the coordinate space of the image; used by
  <gdImagePolygon>, <gdImageOpenPolygon> and <gdImageFilledPolygon>
  for polygon drawing.

  > typedef struct {
  >     int x, y;
  > } gdPoint, *gdPointPtr;

*/
typedef struct {
    int x, y;
}
gdPoint, *gdPointPtr;

/**
 * Typedef: gdRect
 *
 * A rectangle in the coordinate space of the image
 *
 * Members:
 *   x      - The x-coordinate of the upper left corner.
 *   y      - The y-coordinate of the upper left corner.
 *   width  - The width.
 *   height - The height.
 *
 * Typedef: gdRectPtr
 *
 * A pointer to a <gdRect>
 */
typedef struct {
    int x, y;
    int width, height;
}
gdRect, *gdRectPtr;


/**
 * Group: Transform
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
 * See also:
 *  - <gdImageSetInterpolationMethod>
 *  - <gdImageGetInterpolationMethod>
 */
typedef enum {
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


/*
   Group: Types

   typedef: gdImage

   typedef: gdImagePtr

   The data structure in which gd stores images. <gdImageCreate>,
   <gdImageCreateTrueColor> and the various image file-loading functions
   return a pointer to this type, and the other functions expect to
   receive a pointer to this type as their first argument.

   *gdImagePtr* is a pointer to *gdImage*.

   See also:
     <Accessor Macros>

   (Previous versions of this library encouraged directly manipulating
   the contents of the struct but we are attempting to move away from
   this practice so the fields are no longer documented here.  If you
   need to poke at the internals of this struct, feel free to look at
   *gd.h*.)
*/

typedef struct gdImageStruct
{
    /* Truecolor flag and pixels. New 2.0 fields appear here at the
       end to minimize breakage of existing object code. */
    int trueColor;
    int **tpixels;
    int sx;
    int sy;

    /* 2.0.12: simple clipping rectangle. These values
      must be checked for safety when set; please use
      gdImageSetClip */
    int cx1;
    int cy1;
    int cx2;
    int cy2;

    gdInterpolationMethod interpolation_id;
    interpolation_method interpolation;

    int transparent;

    /* Should alpha channel be copied, or applied, each time a
       pixel is drawn? This applies to truecolor images only.
       No attempt is made to alpha-blend in palette images,
       even if semitransparent palette entries exist.
       To do that, build your image as a truecolor image,
       then quantize down to 8 bits. */
    int alphaBlendingFlag;
    /* Should the alpha channel of the image be saved? This affects
       PNG at the moment; other future formats may also
       have that capability. JPEG doesn't. */
    int saveAlphaFlag;

    /* There should NEVER BE ACCESSOR MACROS FOR ITEMS BELOW HERE, so this
       part of the structure can be safely changed in new releases. */

    /* 2.0.12: anti-aliased globals. 2.0.26: just a few vestiges after
      switching to the fast, memory-cheap implementation from PHP-gd. */
    int AA;
    int AA_color;
    int AA_dont_blend;

    /* New in 2.0: thickness of line. Initialized to 1. */
    int thick;

    /* 2.1.0: allows to specify resolution in dpi */
    unsigned int res_x;
    unsigned int res_y;

#if 0
    /* Palette-based image pixels */
    unsigned char **pixels;
    /* These are valid in palette images only. See also
       'alpha', which appears later in the structure to
       preserve binary backwards compatibility */
    int colorsTotal;
    int red[gdMaxColors];
    int green[gdMaxColors];
    int blue[gdMaxColors];
    int open[gdMaxColors];
    /* For backwards compatibility, this is set to the
       first palette entry with 100% transparency,
       and is also set and reset by the
       gdImageColorTransparent function. Newer
       applications can allocate palette entries
       with any desired level of transparency; however,
       bear in mind that many viewers, notably
       many web browsers, fail to implement
       full alpha channel for PNG and provide
       support for full opacity or transparency only. */
    int *polyInts;
    int polyAllocated;
    struct gdImageStruct *brush;
    struct gdImageStruct *tile;
    int brushColorMap[gdMaxColors];
    int tileColorMap[gdMaxColors];
    int styleLength;
    int stylePos;
    int *style;
    int interlace;
    /* New in 2.0: alpha channel for palettes. Note that only
       Macintosh Internet Explorer and (possibly) Netscape 6
       really support multiple levels of transparency in
       palettes, to my knowledge, as of 2/15/01. Most
       common browsers will display 100% opaque and
       100% transparent correctly, and do something
       unpredictable and/or undesirable for levels
       in between. TBB */
    int alpha[gdMaxColors];

    /* Selects quantization method, see gdImageTrueColorToPaletteSetMethod() and gdPaletteQuantizationMethod enum. */
    int paletteQuantizationMethod;
    /* speed/quality trade-off. 1 = best quality, 10 = best speed. 0 = method-specific default.
       Applicable to GD_QUANT_LIQ and GD_QUANT_NEUQUANT. */
    int paletteQuantizationSpeed;
    /* Image will remain true-color if conversion to palette cannot achieve given quality.
       Value from 1 to 100, 1 = ugly, 100 = perfect. Applicable to GD_QUANT_LIQ.*/
    int paletteQuantizationMinQuality;
    /* Image will use minimum number of palette colors needed to achieve given quality. Must be higher than paletteQuantizationMinQuality
       Value from 1 to 100, 1 = ugly, 100 = perfect. Applicable to GD_QUANT_LIQ.*/
    int paletteQuantizationMaxQuality;
#endif

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
gdImagePtr gdImageCreateTrueColor (int sx, int sy);

gdImagePtr gdImageClone (gdImagePtr src);
int gdImageSetInterpolationMethod(gdImagePtr im, gdInterpolationMethod id);
gdInterpolationMethod gdImageGetInterpolationMethod(gdImagePtr im);
void gdImageDestroy (gdImagePtr im);

gdImagePtr gdImageRotate90(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate180(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate270(gdImagePtr src, int ignoretransparent);

#endif // GD_IMAGE_H


