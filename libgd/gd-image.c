#include "gd-image.h"

#include "gd-helpers.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

/**
 * Group: Colors
 *
 * Colors are always of type int which is supposed to be at least 32 bit large.
 *
 * Kinds of colors:
 *   true colors     - ARGB values where the alpha channel is stored as most
 *                     significant, and the blue channel as least significant
 *                     byte. Note that the alpha channel only uses the 7 least
 *                     significant bits.
 *                     Don't rely on the internal representation, though, and
 *                     use <gdTrueColorAlpha> to compose a truecolor value, and
 *                     <gdTrueColorGetAlpha>, <gdTrueColorGetRed>,
 *                     <gdTrueColorGetGreen> and <gdTrueColorGetBlue> to access
 *                     the respective channels.
 *   palette indexes - The index of a color palette entry (0-255).
 *   special colors  - As listed in the following section.
 *
 * Constants: Special Colors
 *   gdStyled        - use the current style, see <gdImageSetStyle>
 *   gdBrushed       - use the current brush, see <gdImageSetBrush>
 *   gdStyledBrushed - use the current style and brush
 *   gdTiled         - use the current tile, see <gdImageSetTile>
 *   gdTransparent   - indicate transparency, what is not the same as the
 *                     transparent color index; used for lines only
 *   gdAntiAliased   - draw anti aliased
 */

#define gdStyled (-2)
#define gdBrushed (-3)
#define gdStyledBrushed (-4)
#define gdTiled (-5)
#define gdTransparent (-6)
#define gdAntiAliased (-7)


/*
    Function: gdImageBoundsSafe
*/
int gdImageBoundsSafe (gdImagePtr im, int x, int y)
{
    return gdImageBoundsSafeMacro (im, x, y);
}

/*
    Function: gdImageCreateTrueColor

      <gdImageCreateTrueColor> is called to create truecolor images,
      with an essentially unlimited number of colors. Invoke
      <gdImageCreateTrueColor> with the x and y dimensions of the
      desired image. <gdImageCreateTrueColor> returns a <gdImagePtr>
      to the new image, or NULL if unable to allocate the image. The
      image must eventually be destroyed using <gdImageDestroy>().

      Truecolor images are always filled with black at creation
      time. There is no concept of a "background" color index.

    Parameters:

        sx - The image width.
        sy - The image height.

    Returns:

        A pointer to the new image or NULL if an error occurred.

    Example:
      (start code)

      gdImagePtr im;
      im = gdImageCreateTrueColor(64, 64);
      // ... Use the image ...
      gdImageDestroy(im);

      (end code)

    See Also:

        <gdImageCreateTrueColor>

*/
gdImagePtr gdImageCreateTrueColor (int sx, int sy)
{
    int i;
    gdImagePtr im;

    if (overflow2(sx, sy)) {
        return NULL;
    }
    if (overflow2(sizeof (int *), sy)) {
        return 0;
    }
    if (overflow2(sizeof(int), sx)) {
        return NULL;
    }

    im = (gdImage *) gdMalloc (sizeof (gdImage));
    if (!im) {
        return 0;
    }

    memset (im, 0, sizeof (gdImage));

    im->tpixels = (int **) gdMalloc (sizeof (int *) * sy);

    if (!im->tpixels) {
        gdFree(im);
        return 0;
    }

//    im->polyInts = 0;
//    im->polyAllocated = 0;
//    im->brush = 0;
//    im->tile = 0;
//    im->style = 0;


    for (i = 0; (i < sy); i++) {
        im->tpixels[i] = (int *) gdCalloc (sx, sizeof (int));
        if (!im->tpixels[i]) {
            /* 2.0.34 */
            i--;
            while (i >= 0) {
                gdFree(im->tpixels[i]);
                i--;
            }
            gdFree(im->tpixels);
            gdFree(im);
            return 0;
        }
    }

    im->sx = sx;
    im->sy = sy;
    im->transparent = (-1);
//    im->interlace = 0;

    /* 2.0.2: alpha blending is now on by default, and saving of alpha is
       off by default. This allows font antialiasing to work as expected
       on the first try in JPEGs -- quite important -- and also allows
       for smaller PNGs when saving of alpha channel is not really
       desired, which it usually isn't! */
    im->saveAlphaFlag = 0;
    im->alphaBlendingFlag = 1;
    im->thick = 1;
    im->AA = 0;
    im->cx1 = 0;
    im->cy1 = 0;
    im->cx2 = im->sx - 1;
    im->cy2 = im->sy - 1;
    im->res_x = GD_RESOLUTION;
    im->res_y = GD_RESOLUTION;
    im->interpolation = NULL;
    im->interpolation_id = GD_BILINEAR_FIXED;

    return im;
}

/*
  Function: gdImageDestroy

    <gdImageDestroy> is used to free the memory associated with an
    image. It is important to invoke <gdImageDestroy> before exiting
    your program or assigning a new image to a <gdImagePtr> variable.

  Parameters:

    im  - Pointer to the gdImage to delete.

  Returns:

    Nothing.

  Example:
    (start code)

    gdImagePtr im;
    im = gdImageCreate(10, 10);
    // ... Use the image ...
    // Now destroy it
    gdImageDestroy(im);

    (end code)

*/

void gdImageDestroy (gdImagePtr im)
{
    int i;

    //if (im->pixels) {
    //    for (i = 0; (i < im->sy); i++) {
    //        gdFree (im->pixels[i]);
    //    }
    //    gdFree (im->pixels);
    //}

    if (im->tpixels)
    {
        for (i = 0; (i < im->sy); i++)
        {
            gdFree (im->tpixels[i]);
        }
        gdFree (im->tpixels);
    }

    //if (im->polyInts) {
    //    gdFree (im->polyInts);
    //}
    //if (im->style) {
    //    gdFree (im->style);
    //}
    gdFree (im);
}

/**
 * Function: gdImageClone
 *
 * Clones an image
 *
 * Creates an exact duplicate of the given image.
 *
 * Parameters:
 *   src - The source image.
 *
 * Returns:
 *   The cloned image on success, NULL on failure.
 */
gdImagePtr gdImageClone (gdImagePtr src)
{
    gdImagePtr dst = {0};
    register int i, x;

    dst = gdImageCreateTrueColor(src->sx , src->sy);

    if (dst == NULL) {
        return NULL;
    }

    for (i = 0; i < src->sy; i++) {
        for (x = 0; x < src->sx; x++) {
            dst->tpixels[i][x] = src->tpixels[i][x];
        }
    }

    //dst->interlace   = src->interlace;

    dst->alphaBlendingFlag = src->alphaBlendingFlag;
    dst->saveAlphaFlag     = src->saveAlphaFlag;
    dst->AA                = src->AA;
    dst->AA_color          = src->AA_color;
    dst->AA_dont_blend     = src->AA_dont_blend;

    dst->cx1 = src->cx1;
    dst->cy1 = src->cy1;
    dst->cx2 = src->cx2;
    dst->cy2 = src->cy2;

    dst->res_x = src->res_x;
    dst->res_y = src->res_y;

    dst->interpolation_id = src->interpolation_id;
    dst->interpolation    = src->interpolation;

    //	dst->paletteQuantizationMethod     = src->paletteQuantizationMethod;
    //	dst->paletteQuantizationSpeed      = src->paletteQuantizationSpeed;
    //	dst->paletteQuantizationMinQuality = src->paletteQuantizationMinQuality;
    //	dst->paletteQuantizationMaxQuality = src->paletteQuantizationMaxQuality;

//	if (src->brush) {
//		dst->brush = gdImageClone(src->brush);
//	}

//	if (src->tile) {
//		dst->tile = gdImageClone(src->tile);
//	}

//	if (src->style) {
//		gdImageSetStyle(dst, src->style, src->styleLength);
//		dst->stylePos = src->stylePos;
//	}

//	for (i = 0; i < gdMaxColors; i++) {
//		dst->brushColorMap[i] = src->brushColorMap[i];
//		dst->tileColorMap[i] = src->tileColorMap[i];
//	}

//	if (src->polyAllocated > 0 && overflow2(sizeof(int), src->polyAllocated) == 0) {
//		dst->polyInts = gdMalloc (sizeof (int) * src->polyAllocated);
//		dst->polyAllocated = src->polyAllocated;
//		for (i = 0; i < src->polyAllocated; i++) {
//			dst->polyInts[i] = src->polyInts[i];
//		}
//	}

    return dst;
}



#define gd_set_alpha(r, g, b, a) (((a) << 24) + \
                                  ((r) << 16) + \
                                  ((g) <<  8) + \
                                   (b))

#define gd_get_red(c)   (((c) & 0xFF0000) >> 16)
#define gd_get_green(c) (((c) & 0x00FF00) >> 8)
#define gd_get_blue(c)   ((c) & 0x0000FF)

#define gd_get_alpha(c) (((c) & 0xFF000000) >> 24)

gdImage* pixbuf_to_gd(GdkPixbuf *pixbuf)
{
    if (!pixbuf)
        return NULL;

    int sx = gdk_pixbuf_get_width(pixbuf);
    int sy = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    gboolean has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);

    if (has_alpha)
        printf("alpha\n");

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    gdImage *img = gdImageCreateTrueColor(sx, sy);

    for (int y = 0; y < sy; ++y)
    {
        for (int x = 0; x < sx; ++x)
        {
            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            img->tpixels[y][x] = gd_set_alpha(p[0], p[1], p[2], 255);
        }
    }

    return img;
}

GdkPixbuf* gd_to_pixbuf(gdImage *src)
{
    g_return_val_if_fail(src != NULL, NULL);

    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8,
                                       src->sx, src->sy);

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    for (int y = 0; y < src->sy; ++y)
    {
        for (int x = 0; x < src->sx; ++x)
        {
            int c = src->tpixels[y][x];

            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            p[0] = gd_get_red(c);
            p[1] = gd_get_green(c);
            p[2] = gd_get_blue(c);
            p[3] = 255; //gd_get_alpha(c);
        }
    }

    return pixbuf;
}






/**
 * Function: gdAffineRotate
 * Set up a rotation affine transform.
 *
 * Like the other angle in libGD, in which increasing y moves
 * downward, this is a counterclockwise rotation.
 *
 * Parameters:
 * 	dst - Where to store the resulting affine transform
 * 	angle - Rotation angle in degrees
 *
 * Returns:
 *  GD_TRUE on success or GD_FALSE
 */
int gdAffineRotate (double dst[6], const double angle)
{
    const double sin_t = sin (angle * M_PI / 180.0);
    const double cos_t = cos (angle * M_PI / 180.0);

    dst[0] = cos_t;
    dst[1] = sin_t;
    dst[2] = -sin_t;
    dst[3] = cos_t;
    dst[4] = 0;
    dst[5] = 0;
    return true;
}

/**
 * Function: gdAffineApplyToPointF
 *  Applies an affine transformation to a point (floating point
 *  gdPointF)
 *
 *
 * Parameters:
 * 	dst - Where to store the resulting point
 *  affine - Source Point
 *  flip_horz - affine matrix
 *
 * Returns:
 *  GD_TRUE if the affine is rectilinear or GD_FALSE
 */
int gdAffineApplyToPointF (gdPointFPtr dst, const gdPointFPtr src,
          const double affine[6])
{
    double x = src->x;
    double y = src->y;
    dst->x = x * affine[0] + y * affine[2] + affine[4];
    dst->y = x * affine[1] + y * affine[3] + affine[5];
    return true;
}

/**
 * Function: gdImageSetClip
 *
 * Sets the clipping rectangle
 *
 * The clipping rectangle restricts the drawing area for following drawing
 * operations.
 *
 * Parameters:
 *   im - The image.
 *   x1 - The x-coordinate of the upper left corner.
 *   y1 - The y-coordinate of the upper left corner.
 *   x2 - The x-coordinate of the lower right corner.
 *   y2 - The y-coordinate of the lower right corner.
 *
 * See also:
 *   - <gdImageGetClip>
 */
void gdImageSetClip (gdImagePtr im, int x1, int y1, int x2, int y2)
{
    if (x1 < 0) {
        x1 = 0;
    }
    if (x1 >= im->sx) {
        x1 = im->sx - 1;
    }
    if (x2 < 0) {
        x2 = 0;
    }
    if (x2 >= im->sx) {
        x2 = im->sx - 1;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (y1 >= im->sy) {
        y1 = im->sy - 1;
    }
    if (y2 < 0) {
        y2 = 0;
    }
    if (y2 >= im->sy) {
        y2 = im->sy - 1;
    }
    im->cx1 = x1;
    im->cy1 = y1;
    im->cx2 = x2;
    im->cy2 = y2;
}

/**
 * Function: gdImageGetClip
 *
 * Gets the current clipping rectangle
 *
 * Parameters:
 *   im - The image.
 *   x1P - (out) The x-coordinate of the upper left corner.
 *   y1P - (out) The y-coordinate of the upper left corner.
 *   x2P - (out) The x-coordinate of the lower right corner.
 *   y2P - (out) The y-coordinate of the lower right corner.
 *
 * See also:
 *   - <gdImageSetClip>
 */
void gdImageGetClip (gdImagePtr im, int *x1P, int *y1P, int *x2P, int *y2P)
{
    *x1P = im->cx1;
    *y1P = im->cy1;
    *x2P = im->cx2;
    *y2P = im->cy2;
}

/**
 * Function: gdAffineInvert
 *  Find the inverse of an affine transformation.
 *
 * All non-degenerate affine transforms are invertible. Applying the
 * inverted matrix will restore the original values. Multiplying <src>
 * by <dst> (commutative) will return the identity affine (rounding
 * error possible).
 *
 * Parameters:
 * 	dst - Where to store the resulting affine transform
 *  src_affine - Original affine matrix
 *  flip_horz - Whether or not to flip horizontally
 *  flip_vert - Whether or not to flip vertically
 *
 * See also:
 *  <gdAffineIdentity>
 *
 * Returns:
 *  GD_TRUE on success or GD_FALSE on failure
 */
int gdAffineInvert (double dst[6], const double src[6])
{
    double r_det = (src[0] * src[3] - src[1] * src[2]);

    if (!isfinite(r_det)) {
        return false;
    }
    if (r_det == 0) {
        return false;
    }

    r_det = 1.0 / r_det;
    dst[0] = src[3] * r_det;
    dst[1] = -src[1] * r_det;
    dst[2] = -src[2] * r_det;
    dst[3] = src[0] * r_det;
    dst[4] = -src[4] * dst[0] - src[5] * dst[2];
    dst[5] = -src[4] * dst[1] - src[5] * dst[3];

    return true;
}

/**
 * Function: gdAffineTranslate
 * Set up a translation matrix.
 *
 * Parameters:
 * 	dst - Where to store the resulting affine transform
 * 	offset_x - Horizontal translation amount
 * 	offset_y - Vertical translation amount
 *
 * Returns:
 *  GD_TRUE on success or GD_FALSE
 */
int gdAffineTranslate (double dst[6], const double offset_x, const double offset_y)
{
    dst[0] = 1;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 1;
    dst[4] = offset_x;
    dst[5] = offset_y;
    return true;
}

/**
 * Function: gdAffineConcat
 * Concat (Multiply) two affine transformation matrices.
 *
 * Concats two affine transforms together, i.e. the result
 * will be the equivalent of doing first the transformation m1 and then
 * m2. All parameters can be the same matrix (safe to call using
 * the same array for all three arguments).
 *
 * Parameters:
 * 	dst - Where to store the resulting affine transform
 *  m1 - First affine matrix
 *  m2 - Second affine matrix
 *
 * Returns:
 *  GD_TRUE on success or GD_FALSE
 */
int gdAffineConcat (double dst[6], const double m1[6], const double m2[6])
{
    double dst0, dst1, dst2, dst3, dst4, dst5;

    dst0 = m1[0] * m2[0] + m1[1] * m2[2];
    dst1 = m1[0] * m2[1] + m1[1] * m2[3];
    dst2 = m1[2] * m2[0] + m1[3] * m2[2];
    dst3 = m1[2] * m2[1] + m1[3] * m2[3];
    dst4 = m1[4] * m2[0] + m1[5] * m2[2] + m2[4];
    dst5 = m1[4] * m2[1] + m1[5] * m2[3] + m2[5];
    dst[0] = dst0;
    dst[1] = dst1;
    dst[2] = dst2;
    dst[3] = dst3;
    dst[4] = dst4;
    dst[5] = dst5;
    return true;
}

/**
 * Function: gdAlphaBlend
 *
 * Blend two colors
 *
 * Parameters:
 *   dst - The color to blend onto.
 *   src - The color to blend.
 *
 * See also:
 *   - <gdImageAlphaBlending>
 *   - <gdLayerOverlay>
 *   - <gdLayerMultiply>
 */
int gdAlphaBlend (int dst, int src)
{
    int src_alpha = gdTrueColorGetAlpha(src);
    int dst_alpha, alpha, red, green, blue;
    int src_weight, dst_weight, tot_weight;

    /* -------------------------------------------------------------------- */
    /*      Simple cases we want to handle fast.                            */
    /* -------------------------------------------------------------------- */
    if( src_alpha == gdAlphaOpaque )
        return src;

    dst_alpha = gdTrueColorGetAlpha(dst);
    if( src_alpha == gdAlphaTransparent )
        return dst;
    if( dst_alpha == gdAlphaTransparent )
        return src;

    /* -------------------------------------------------------------------- */
    /*      What will the source and destination alphas be?  Note that      */
    /*      the destination weighting is substantially reduced as the       */
    /*      overlay becomes quite opaque.                                   */
    /* -------------------------------------------------------------------- */
    src_weight = gdAlphaTransparent - src_alpha;
    dst_weight = (gdAlphaTransparent - dst_alpha) * src_alpha / gdAlphaMax;
    tot_weight = src_weight + dst_weight;

    /* -------------------------------------------------------------------- */
    /*      What red, green and blue result values will we use?             */
    /* -------------------------------------------------------------------- */
    alpha = src_alpha * dst_alpha / gdAlphaMax;

    red = (gdTrueColorGetRed(src) * src_weight
           + gdTrueColorGetRed(dst) * dst_weight) / tot_weight;
    green = (gdTrueColorGetGreen(src) * src_weight
             + gdTrueColorGetGreen(dst) * dst_weight) / tot_weight;
    blue = (gdTrueColorGetBlue(src) * src_weight
            + gdTrueColorGetBlue(dst) * dst_weight) / tot_weight;

    /* -------------------------------------------------------------------- */
    /*      Return merged result.                                           */
    /* -------------------------------------------------------------------- */
    return ((alpha << 24) + (red << 16) + (green << 8) + blue);
}

/* Apply 'overlay' effect - background pixels are colourised by the foreground colour */
static int gdAlphaOverlayColor (int src, int dst, int max )
{
    dst = dst << 1;
    if( dst > max ) {
        /* in the "light" zone */
        return dst + (src << 1) - (dst * src / max) - max;
    } else {
        /* in the "dark" zone */
        return dst * src / max;
    }
}


/**
 * Function: gdLayerOverlay
 *
 * Overlay two colors
 *
 * Parameters:
 *   dst - The color to overlay onto.
 *   src - The color to overlay.
 *
 * See also:
 *   - <gdImageAlphaBlending>
 *   - <gdAlphaBlend>
 *   - <gdLayerMultiply>
 */
int gdLayerOverlay (int dst, int src)
{
    int a1, a2;
    a1 = gdAlphaMax - gdTrueColorGetAlpha(dst);
    a2 = gdAlphaMax - gdTrueColorGetAlpha(src);
    return ( ((gdAlphaMax - a1*a2/gdAlphaMax) << 24) +
        (gdAlphaOverlayColor( gdTrueColorGetRed(src), gdTrueColorGetRed(dst), gdRedMax ) << 16) +
        (gdAlphaOverlayColor( gdTrueColorGetGreen(src), gdTrueColorGetGreen(dst), gdGreenMax ) << 8) +
        (gdAlphaOverlayColor( gdTrueColorGetBlue(src), gdTrueColorGetBlue(dst), gdBlueMax ))
        );
}

/**
 *	Function: gdImageAlphaBlending
 *
 *	Set the effect for subsequent drawing operations
 *
 *	Note that the effect is used for truecolor images only.
 *
 * Parameters:
 *   im               - The image.
 *   alphaBlendingArg - The effect.
 *
 * See also:
 *   - <Effects>
 */
void gdImageAlphaBlending (gdImagePtr im, int alphaBlendingArg)
{
    im->alphaBlendingFlag = alphaBlendingArg;
}


// rotate ---------------------------------------------------------------------

typedef int (BGD_STDCALL *FuncPtr)(gdImagePtr, int, int);

#if 0
/* Rotates an image by 270 degrees (counter clockwise) */
gdImagePtr gdImageRotate270 (gdImagePtr src, int ignoretransparent)
{
    int uY, uX;
    //int c,r,g,b,a;
    gdImagePtr dst;
    FuncPtr f;

    f = gdImageGetTrueColorPixel;

    dst = gdImageCreateTrueColor (src->sy, src->sx);

    if (dst != NULL) {
        int old_blendmode = dst->alphaBlendingFlag;
        dst->alphaBlendingFlag = 0;
        dst->saveAlphaFlag = 1;
        dst->transparent = src->transparent;

//        gdImagePaletteCopy (dst, src);

        for (uY = 0; uY<src->sy; uY++) {
            for (uX = 0; uX<src->sx; uX++) {
                c = f (src, uX, uY);

//                if (!src->trueColor) {
//                    r = gdImageRed(src,c);
//                    g = gdImageGreen(src,c);
//                    b = gdImageBlue(src,c);
//                    a = gdImageAlpha(src,c);
//                    c = gdTrueColorAlpha(r, g, b, a);
//                }

                if (ignoretransparent && c == dst->transparent) {
                    gdImageSetPixel(dst, (dst->sx - uY - 1), uX, dst->transparent);
                } else {
                    gdImageSetPixel(dst, (dst->sx - uY - 1), uX, c);
                }
            }
        }
        dst->alphaBlendingFlag = old_blendmode;
    }

    return dst;
}

/* Rotates an image by 90 degrees (counter clockwise) */
gdImagePtr gdImageRotate90 (gdImagePtr src, int ignoretransparent)
{
    int uY, uX;
    int c,r,g,b,a;
    gdImagePtr dst;
    FuncPtr f;

    if (src->trueColor) {
        f = gdImageGetTrueColorPixel;
    } else {
        f = gdImageGetPixel;
    }
    dst = gdImageCreateTrueColor(src->sy, src->sx);
    if (dst != NULL) {
        int old_blendmode = dst->alphaBlendingFlag;
        dst->alphaBlendingFlag = 0;
        dst->saveAlphaFlag = 1;
        dst->transparent = src->transparent;

        gdImagePaletteCopy (dst, src);

        for (uY = 0; uY<src->sy; uY++) {
            for (uX = 0; uX<src->sx; uX++) {
                c = f (src, uX, uY);
                if (!src->trueColor) {
                    r = gdImageRed(src,c);
                    g = gdImageGreen(src,c);
                    b = gdImageBlue(src,c);
                    a = gdImageAlpha(src,c);
                    c = gdTrueColorAlpha(r, g, b, a);
                }
                if (ignoretransparent && c == dst->transparent) {
                    gdImageSetPixel(dst, uY, (dst->sy - uX - 1), dst->transparent);
                } else {
                    gdImageSetPixel(dst, uY, (dst->sy - uX - 1), c);
                }
            }
        }
        dst->alphaBlendingFlag = old_blendmode;
    }

    return dst;
}

/* Rotates an image by 180 degrees (counter clockwise) */
gdImagePtr gdImageRotate180 (gdImagePtr src, int ignoretransparent)
{
    int uY, uX;
    int c,r,g,b,a;
    gdImagePtr dst;
    FuncPtr f;

    if (src->trueColor) {
        f = gdImageGetTrueColorPixel;
    } else {
        f = gdImageGetPixel;
    }
    dst = gdImageCreateTrueColor(src->sx, src->sy);

    if (dst != NULL) {
        int old_blendmode = dst->alphaBlendingFlag;
        dst->alphaBlendingFlag = 0;
        dst->saveAlphaFlag = 1;
        dst->transparent = src->transparent;

        gdImagePaletteCopy (dst, src);

        for (uY = 0; uY<src->sy; uY++) {
            for (uX = 0; uX<src->sx; uX++) {
                c = f (src, uX, uY);
                if (!src->trueColor) {
                    r = gdImageRed(src,c);
                    g = gdImageGreen(src,c);
                    b = gdImageBlue(src,c);
                    a = gdImageAlpha(src,c);
                    c = gdTrueColorAlpha(r, g, b, a);
                }

                if (ignoretransparent && c == dst->transparent) {
                    gdImageSetPixel(dst, (dst->sx - uX - 1), (dst->sy - uY - 1), dst->transparent);
                } else {
                    gdImageSetPixel(dst, (dst->sx - uX - 1), (dst->sy - uY - 1), c);
                }
            }
        }
        dst->alphaBlendingFlag = old_blendmode;
    }

    return dst;
}
#endif

#if 0
/*
    Function: gdImageSetPixel
*/
void gdImageSetPixel (gdImagePtr im, int x, int y, int color)
{
    int p;
    switch (color) {
    case gdStyled:
        if (!im->style) {
            /* Refuse to draw if no style is set. */
            return;
        } else {
            p = im->style[im->stylePos++];
        }
        if (p != (gdTransparent)) {
            gdImageSetPixel (im, x, y, p);
        }
        im->stylePos = im->stylePos % im->styleLength;
        break;
    case gdStyledBrushed:
        if (!im->style) {
            /* Refuse to draw if no style is set. */
            return;
        }
        p = im->style[im->stylePos++];
        if ((p != gdTransparent) && (p != 0)) {
            gdImageSetPixel (im, x, y, gdBrushed);
        }
        im->stylePos = im->stylePos % im->styleLength;
        break;
    case gdBrushed:
        gdImageBrushApply (im, x, y);
        break;
    case gdTiled:
        gdImageTileApply (im, x, y);
        break;
    case gdAntiAliased:
        /* This shouldn't happen (2.0.26) because we just call
          gdImageAALine now, but do something sane. */
        gdImageSetPixel(im, x, y, im->AA_color);
        break;
    default:
        if (gdImageBoundsSafeMacro (im, x, y)) {
            if (im->trueColor) {
                switch (im->alphaBlendingFlag) {
                    default:
                    case gdEffectReplace:
                        im->tpixels[y][x] = color;
                        break;
                    case gdEffectAlphaBlend:
                    case gdEffectNormal:
                        im->tpixels[y][x] = gdAlphaBlend(im->tpixels[y][x], color);
                        break;
                    case gdEffectOverlay :
                        im->tpixels[y][x] = gdLayerOverlay(im->tpixels[y][x], color);
                        break;
                    case gdEffectMultiply :
                        im->tpixels[y][x] = gdLayerMultiply(im->tpixels[y][x], color);
                        break;
                }
            } else {
                im->pixels[y][x] = color;
            }
        }
        break;
    }
}
#endif


