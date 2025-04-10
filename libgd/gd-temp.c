
#if 0

// helpers --------------------------------------------------------------------

void *gdReallocEx (void *ptr, size_t size);
void* gdReallocEx (void *ptr, size_t size)
{
    void *newPtr = gd_realloc (ptr, size);
    if (!newPtr && ptr)
        gdFree(ptr);
    return newPtr;
}


// image ----------------------------------------------------------------------

void gdImageSetPixel (gdImagePtr im, int x, int y, int color);

im->polyInts = 0;
im->polyAllocated = 0;
im->brush = 0;
im->tile = 0;
im->style = 0;
im->interlace = 0;

    dst->paletteQuantizationMethod     = src->paletteQuantizationMethod;
    dst->paletteQuantizationSpeed      = src->paletteQuantizationSpeed;
    dst->paletteQuantizationMinQuality = src->paletteQuantizationMinQuality;
    dst->paletteQuantizationMaxQuality = src->paletteQuantizationMaxQuality;

if (src->brush) {
    dst->brush = gdImageClone(src->brush);
}

if (src->tile) {
    dst->tile = gdImageClone(src->tile);
}

if (src->style) {
    gdImageSetStyle(dst, src->style, src->styleLength);
    dst->stylePos = src->stylePos;
}

for (i = 0; i < gdMaxColors; i++) {
    dst->brushColorMap[i] = src->brushColorMap[i];
    dst->tileColorMap[i] = src->tileColorMap[i];
}

if (src->polyAllocated > 0 && overflow2(sizeof(int), src->polyAllocated) == 0) {
    dst->polyInts = gdMalloc (sizeof (int) * src->polyAllocated);
    dst->polyAllocated = src->polyAllocated;
    for (i = 0; i < src->polyAllocated; i++) {
        dst->polyInts[i] = src->polyInts[i];
    }
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

void gdImageGetClip(gdImagePtr im, int *x1P, int *y1P, int *x2P, int *y2P);
void gdImageSetClip(gdImagePtr im, int x1, int y1, int x2, int y2);
void gdImageAlphaBlending (gdImagePtr im, int alphaBlendingArg);


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

// rotate ---------------------------------------------------------------------

typedef int (BGD_STDCALL *FuncPtr)(gdImagePtr, int, int);

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


// resize ---------------------------------------------------------------------

static gdImagePtr gdImageRotateNearestNeighbour(gdImagePtr src,
                                                const float degrees,
                                                const int bgColor);
static gdImagePtr gdImageRotateGeneric(gdImagePtr src, const float degrees,
                                       const int bgColor);

#ifdef FUNCTION_NOT_USED_YET

// CubicConvolution filter, default radius 3
static double filter_cubic_convolution(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    const double x2 = x1 * x1;
    const double x2_x = x2 * x;
    ARG_NOT_USED(support);
    if (x <= 1.0) return ((4.0 / 3.0)* x2_x - (7.0 / 3.0) * x2 + 1.0);
    if (x <= 2.0) return (- (7.0 / 12.0) * x2_x + 3 * x2 - (59.0 / 12.0) * x + 2.5);
    if (x <= 3.0) return ( (1.0/12.0) * x2_x - (2.0 / 3.0) * x2 + 1.75 * x - 1.5);
    return 0;
}
#endif

// keep it for future usage for affine copy over an existing image, targetting fix for 2.2.2
#ifdef FUNCTION_NOT_USED_YET
// Copied from upstream's libgd
static inline int _color_blend (const int dst, const int src)
{
    const int src_alpha = gdTrueColorGetAlpha(src);

    if( src_alpha == gdAlphaOpaque ) {
        return src;
    } else {
        const int dst_alpha = gdTrueColorGetAlpha(dst);

        if( src_alpha == gdAlphaTransparent ) return dst;
        if( dst_alpha == gdAlphaTransparent ) {
            return src;
        } else {
            register int alpha, red, green, blue;
            const int src_weight = gdAlphaTransparent - src_alpha;
            const int dst_weight = (gdAlphaTransparent - dst_alpha) * src_alpha / gdAlphaMax;
            const int tot_weight = src_weight + dst_weight;

            alpha = src_alpha * dst_alpha / gdAlphaMax;

            red = (gdTrueColorGetRed(src) * src_weight
                   + gdTrueColorGetRed(dst) * dst_weight) / tot_weight;
            green = (gdTrueColorGetGreen(src) * src_weight
                   + gdTrueColorGetGreen(dst) * dst_weight) / tot_weight;
            blue = (gdTrueColorGetBlue(src) * src_weight
                   + gdTrueColorGetBlue(dst) * dst_weight) / tot_weight;

            return ((alpha << 24) + (red << 16) + (green << 8) + blue);
        }
    }
}

static inline int _setEdgePixel(const gdImagePtr src, unsigned int x, unsigned int y, gdFixed coverage, const int bgColor)
{
    const gdFixed f_127 = gd_itofx(127);
    register int c = src->tpixels[y][x];
    c = c | (( (int) (gd_fxtof(gd_mulfx(coverage, f_127)) + 50.5f)) << 24);
    return _color_blend(bgColor, c);
}
#endif

// palette

#define colorIndex2RGBA(c) gdTrueColorAlpha(im->red[(c)], im->green[(c)], im->blue[(c)], im->alpha[(c)])
#define colorIndex2RGBcustomA(c, a) gdTrueColorAlpha(im->red[(c)], im->green[(c)], im->blue[(c)], im->alpha[(a)])
static inline int getPixelOverflowPalette(gdImagePtr im, const int x, const int y, const int bgColor  /* 31bit ARGB TC */)
{
    if (gdImageBoundsSafe(im, x, y)) {
        const int c = im->pixels[y][x];
        if (c == im->transparent) {
            return bgColor == -1 ? gdTrueColorAlpha(0, 0, 0, 127) : bgColor;
        }
        return colorIndex2RGBA(c);
    } else {
        return bgColor;  // 31bit ARGB TC
    }
}

static gdImagePtr gdImageScaleBilinearPalette(gdImagePtr im, const unsigned int new_width, const unsigned int new_height)
{
    long _width = MAX(1, new_width);
    long _height = MAX(1, new_height);
    float dx = (float)gdImageSX(im) / (float)_width;
    float dy = (float)gdImageSY(im) / (float)_height;
    gdFixed f_dx = gd_ftofx(dx);
    gdFixed f_dy = gd_ftofx(dy);
    gdFixed f_1 = gd_itofx(1);

    int dst_offset_h;
    int dst_offset_v = 0;
    long i;
    gdImagePtr new_img;
    const int transparent = im->transparent;

    new_img = gdImageCreateTrueColor(new_width, new_height);
    if (new_img == NULL) {
        return NULL;
    }

    if (transparent < 0) {
        // uninitialized
        new_img->transparent = -1;
    } else {
        new_img->transparent = gdTrueColorAlpha(im->red[transparent], im->green[transparent], im->blue[transparent], im->alpha[transparent]);
    }

    for (i=0; i < _height; i++) {
        long j;
        const gdFixed f_i = gd_itofx(i);
        const gdFixed f_a = gd_mulfx(f_i, f_dy);
        register long m = gd_fxtoi(f_a);

        dst_offset_h = 0;

        for (j=0; j < _width; j++) {
            // Update bitmap
            gdFixed f_j = gd_itofx(j);
            gdFixed f_b = gd_mulfx(f_j, f_dx);

            const long n = gd_fxtoi(f_b);
            gdFixed f_f = f_a - gd_itofx(m);
            gdFixed f_g = f_b - gd_itofx(n);

            const gdFixed f_w1 = gd_mulfx(f_1-f_f, f_1-f_g);
            const gdFixed f_w2 = gd_mulfx(f_1-f_f, f_g);
            const gdFixed f_w3 = gd_mulfx(f_f, f_1-f_g);
            const gdFixed f_w4 = gd_mulfx(f_f, f_g);
            unsigned int pixel1;
            unsigned int pixel2;
            unsigned int pixel3;
            unsigned int pixel4;
            register gdFixed f_r1, f_r2, f_r3, f_r4,
                    f_g1, f_g2, f_g3, f_g4,
                    f_b1, f_b2, f_b3, f_b4,
                    f_a1, f_a2, f_a3, f_a4;

            // 0 for bgColor; (n,m) is supposed to be valid anyway
            pixel1 = getPixelOverflowPalette(im, n, m, 0);
            pixel2 = getPixelOverflowPalette(im, n + 1, m, pixel1);
            pixel3 = getPixelOverflowPalette(im, n, m + 1, pixel1);
            pixel4 = getPixelOverflowPalette(im, n + 1, m + 1, pixel1);

            f_r1 = gd_itofx(gdTrueColorGetRed(pixel1));
            f_r2 = gd_itofx(gdTrueColorGetRed(pixel2));
            f_r3 = gd_itofx(gdTrueColorGetRed(pixel3));
            f_r4 = gd_itofx(gdTrueColorGetRed(pixel4));
            f_g1 = gd_itofx(gdTrueColorGetGreen(pixel1));
            f_g2 = gd_itofx(gdTrueColorGetGreen(pixel2));
            f_g3 = gd_itofx(gdTrueColorGetGreen(pixel3));
            f_g4 = gd_itofx(gdTrueColorGetGreen(pixel4));
            f_b1 = gd_itofx(gdTrueColorGetBlue(pixel1));
            f_b2 = gd_itofx(gdTrueColorGetBlue(pixel2));
            f_b3 = gd_itofx(gdTrueColorGetBlue(pixel3));
            f_b4 = gd_itofx(gdTrueColorGetBlue(pixel4));
            f_a1 = gd_itofx(gdTrueColorGetAlpha(pixel1));
            f_a2 = gd_itofx(gdTrueColorGetAlpha(pixel2));
            f_a3 = gd_itofx(gdTrueColorGetAlpha(pixel3));
            f_a4 = gd_itofx(gdTrueColorGetAlpha(pixel4));

            {
                const unsigned char red = (unsigned char) gd_fxtoi(gd_mulfx(f_w1, f_r1) + gd_mulfx(f_w2, f_r2) + gd_mulfx(f_w3, f_r3) + gd_mulfx(f_w4, f_r4));
                const unsigned char green = (unsigned char) gd_fxtoi(gd_mulfx(f_w1, f_g1) + gd_mulfx(f_w2, f_g2) + gd_mulfx(f_w3, f_g3) + gd_mulfx(f_w4, f_g4));
                const unsigned char blue = (unsigned char) gd_fxtoi(gd_mulfx(f_w1, f_b1) + gd_mulfx(f_w2, f_b2) + gd_mulfx(f_w3, f_b3) + gd_mulfx(f_w4, f_b4));
                const unsigned char alpha = (unsigned char) gd_fxtoi(gd_mulfx(f_w1, f_a1) + gd_mulfx(f_w2, f_a2) + gd_mulfx(f_w3, f_a3) + gd_mulfx(f_w4, f_a4));

                new_img->tpixels[dst_offset_v][dst_offset_h] = gdTrueColorAlpha(red, green, blue, alpha);
            }

            dst_offset_h++;
        }

        dst_offset_v++;
    }
    return new_img;
}


// rotate

static int gdRotatedImageSize(gdImagePtr src, const float angle, gdRectPtr bbox)
{
    gdRect src_area;
    double m[6];

    gdAffineRotate(m, angle);
    src_area.x = 0;
    src_area.y = 0;
    src_area.width = gdImageSX(src);
    src_area.height = gdImageSY(src);
    if (gdTransformAffineBoundingBox(&src_area, m, bbox) != true) {
        return false;
    }

    return true;
}

static gdImagePtr
gdImageRotateNearestNeighbour(gdImagePtr src, const float degrees,
                              const int bgColor)
{
    float _angle = ((float) (-degrees / 180.0f) * (float)M_PI);
    const int src_w  = gdImageSX(src);
    const int src_h = gdImageSY(src);
    const gdFixed f_0_5 = gd_ftofx(0.5f);
    const gdFixed f_H = gd_itofx(src_h/2);
    const gdFixed f_W = gd_itofx(src_w/2);
    const gdFixed f_cos = gd_ftofx(cos(-_angle));
    const gdFixed f_sin = gd_ftofx(sin(-_angle));

    unsigned int dst_offset_x;
    unsigned int dst_offset_y = 0;
    unsigned int i;
    gdImagePtr dst;
    gdRect bbox;
    unsigned int new_height, new_width;

    gdRotatedImageSize(src, degrees, &bbox);
    new_width = bbox.width;
    new_height = bbox.height;

    dst = gdImageCreateTrueColor(new_width, new_height);
    if (!dst) {
        return NULL;
    }
    dst->saveAlphaFlag = 1;
    for (i = 0; i < new_height; i++) {
        unsigned int j;
        dst_offset_x = 0;
        for (j = 0; j < new_width; j++) {
            gdFixed f_i = gd_itofx((int)i - (int)new_height / 2);
            gdFixed f_j = gd_itofx((int)j - (int)new_width  / 2);
            gdFixed f_m = gd_mulfx(f_j,f_sin) + gd_mulfx(f_i,f_cos) + f_0_5 + f_H;
            gdFixed f_n = gd_mulfx(f_j,f_cos) - gd_mulfx(f_i,f_sin) + f_0_5 + f_W;
            long m = gd_fxtoi(f_m);
            long n = gd_fxtoi(f_n);

            if ((m > 0) && (m < src_h-1) && (n > 0) && (n < src_w-1)) {
                if (dst_offset_y < new_height) {
                    dst->tpixels[dst_offset_y][dst_offset_x++] = src->tpixels[m][n];
                }
            } else {
                if (dst_offset_y < new_height) {
                    dst->tpixels[dst_offset_y][dst_offset_x++] = bgColor;
                }
            }
        }
        dst_offset_y++;
    }
    return dst;
}

static gdImagePtr
gdImageRotateGeneric(gdImagePtr src, const float degrees, const int bgColor)
{
    float _angle = ((float) (-degrees / 180.0f) * (float)M_PI);
    const int src_w  = gdImageSX(src);
    const int src_h = gdImageSY(src);
    const gdFixed f_H = gd_itofx(src_h/2);
    const gdFixed f_W = gd_itofx(src_w/2);
    const gdFixed f_cos = gd_ftofx(cos(-_angle));
    const gdFixed f_sin = gd_ftofx(sin(-_angle));

    unsigned int dst_offset_x;
    unsigned int dst_offset_y = 0;
    unsigned int i;
    gdImagePtr dst;
    unsigned int new_width, new_height;
    gdRect bbox;

    if (bgColor < 0) {
        return NULL;
    }

    if (src->interpolation == NULL) {
        gdImageSetInterpolationMethod(src, GD_DEFAULT);
    }

    gdRotatedImageSize(src, degrees, &bbox);
    new_width = bbox.width;
    new_height = bbox.height;

    dst = gdImageCreateTrueColor(new_width, new_height);
    if (!dst) {
        return NULL;
    }
    dst->saveAlphaFlag = 1;

    for (i = 0; i < new_height; i++) {
        unsigned int j;
        dst_offset_x = 0;
        for (j = 0; j < new_width; j++) {
            gdFixed f_i = gd_itofx((int)i - (int)new_height / 2);
            gdFixed f_j = gd_itofx((int)j - (int)new_width  / 2);
            gdFixed f_m = gd_mulfx(f_j,f_sin) + gd_mulfx(f_i,f_cos) + f_H;
            gdFixed f_n = gd_mulfx(f_j,f_cos) - gd_mulfx(f_i,f_sin)  + f_W;
            long m = gd_fxtoi(f_m);
            long n = gd_fxtoi(f_n);

            if (m < -1 || n < -1 || m >= src_h || n >= src_w ) {
                dst->tpixels[dst_offset_y][dst_offset_x++] = bgColor;
            } else {
                dst->tpixels[dst_offset_y][dst_offset_x++] = getPixelInterpolated(src, gd_fxtod(f_n), gd_fxtod(f_m), bgColor);
            }
        }
        dst_offset_y++;
    }
    return dst;
}

/**
 * Function: gdImageRotateInterpolated
 *
 * Rotate an image
 *
 * Creates a new image, counter-clockwise rotated by the requested angle
 * using the current <gdInterpolationMethod>. Non-square angles will add a
 * border with bgcolor.
 *
 * Parameters:
 *   src     - The source image.
 *   angle   - The angle in degrees.
 *   bgcolor - The color to fill the added background with.
 *
 * Returns:
 *   The rotated image on success, NULL on failure.
 *
 * See also:
 *   - <gdImageCopyRotated>
 */
gdImagePtr gdImageRotateInterpolated(const gdImagePtr src, const float angle, int bgcolor)
{
    /* round to two decimals and keep the 100x multiplication to use it in the common square angles
       case later. Keep the two decimal precisions so smaller rotation steps can be done, useful for
       slow animations, f.e. */
    const int angle_rounded = fmod((int) floorf(angle * 100), 360 * 100);
    gdImagePtr src_tc = src;
    int src_cloned = 0;
    if (src == NULL || bgcolor < 0) {
        return NULL;
    }

    //if (!gdImageTrueColor(src)) {
    //    if (bgcolor < gdMaxColors) {
    //        bgcolor =  gdTrueColorAlpha(src->red[bgcolor], src->green[bgcolor], src->blue[bgcolor], src->alpha[bgcolor]);
    //    }
    //    src_tc = gdImageClone(src);
    //    gdImagePaletteToTrueColor(src_tc);
    //    src_cloned = 1;
    //}

    /* 0 && 90 degrees multiple rotation, 0 rotation simply clones the return image and convert it
       to truecolor, as we must return truecolor image. */
    switch (angle_rounded) {
        case    0:
            return src_cloned ? src_tc : gdImageClone(src);

        case -27000:
        case   9000:
            if (src_cloned) gdImageDestroy(src_tc);
            return gdImageRotate90(src, 0);

        case -18000:
        case  18000:
            if (src_cloned) gdImageDestroy(src_tc);
            return gdImageRotate180(src, 0);

        case  -9000:
        case  27000:
            if (src_cloned) gdImageDestroy(src_tc);
            return gdImageRotate270(src, 0);
    }

    if (src->interpolation_id < 1 || src->interpolation_id > GD_METHOD_COUNT) {
        if (src_cloned) gdImageDestroy(src_tc);
        return NULL;
    }

    switch (src->interpolation_id) {
        case GD_NEAREST_NEIGHBOUR: {
            gdImagePtr res = gdImageRotateNearestNeighbour(src_tc, angle, bgcolor);
            if (src_cloned) gdImageDestroy(src_tc);
            return res;
        }

        case GD_BILINEAR_FIXED:
        case GD_BICUBIC_FIXED:
        default: {
            gdImagePtr res = gdImageRotateGeneric(src_tc, angle, bgcolor);
            if (src_cloned) gdImageDestroy(src_tc);
            return res;
        }
    }
    return NULL;
}


/** Function: getPixelRgbInterpolated
 *   get the index of the image's colors
 *
 * Parameters:
 *  im - Image to draw the transformed image
 *  tcolor - TrueColor
 *
 * Return:
 *  index of colors
 */
#if 0
static int getPixelRgbInterpolated(gdImagePtr im, const int tcolor)
{
    unsigned char r, g, b, a;
    int ct;
    int i;

    b = (unsigned char)(tcolor);
    g = (unsigned char)(tcolor >> 8);
    r = (unsigned char)(tcolor >> 16);
    a = (unsigned char)(tcolor >> 24);

    for (i = 0; i < im->colorsTotal; i++) {
        if (im->red[i] == r && im->green[i] == g && im->blue[i] == b && im->alpha[i] == a) {
            return i;
        }
    }

    ct = im->colorsTotal;
    if (ct == gdMaxColors) {
        return -1;
    }

    im->colorsTotal++;
    im->red[ct] = r;
    im->green[ct] = g;
    im->blue[ct] = b;
    im->alpha[ct] = a;
    im->open[ct] = 0;

    return ct;
}
#endif




// ----------------------------------------------------------------------------

/**
 * Group: Affine Transformation
 **/

int gdAffineRotate (double dst[6], const double angle);

int gdAffineApplyToPointF (gdPointFPtr dst, const gdPointFPtr src, const double affine[6]);
int gdAffineInvert (double dst[6], const double src[6]);
int gdAffineFlip (double dst_affine[6], const double src_affine[6], const int flip_h, const int flip_v);
int gdAffineConcat (double dst[6], const double m1[6], const double m2[6]);

int gdAffineIdentity (double dst[6]);
int gdAffineScale (double dst[6], const double scale_x, const double scale_y);
int gdAffineShearHorizontal (double dst[6], const double angle);
int gdAffineShearVertical(double dst[6], const double angle);
int gdAffineTranslate (double dst[6], const double offset_x, const double offset_y);
double gdAffineExpansion (const double src[6]);
int gdAffineRectilinear (const double src[6]);
int gdAffineEqual (const double matrix1[6], const double matrix2[6]);
int gdTransformAffineGetImage(gdImagePtr *dst, const gdImagePtr src, gdRectPtr src_area, const double affine[6]);
int gdTransformAffineCopy(gdImagePtr dst, int dst_x, int dst_y, const gdImagePtr src, gdRectPtr src_region, const double affine[6]);


 static void gdImageClipRectangle(gdImagePtr im, gdRectPtr r)
{
    int c1x, c1y, c2x, c2y;
    int x1,y1;

    gdImageGetClip(im, &c1x, &c1y, &c2x, &c2y);
    x1 = r->x + r->width - 1;
    y1 = r->y + r->height - 1;
    r->x = CLAMP(r->x, c1x, c2x);
    r->y = CLAMP(r->y, c1y, c2y);
    r->width = CLAMP(x1, c1x, c2x) - r->x + 1;
    r->height = CLAMP(y1, c1y, c2y) - r->y + 1;
}


/**
 * Function: gdTransformAffineGetImage
 *  Applies an affine transformation to a region and return an image
 *  containing the complete transformation.
 *
 * Parameters:
 * 	dst - Pointer to a gdImagePtr to store the created image, NULL when
 *        the creation or the transformation failed
 *  src - Source image
 *  src_area - rectangle defining the source region to transform
 *  dstY - Y position in the destination image
 *  affine - The desired affine transformation
 *
 * Returns:
 *  true if the affine is rectilinear or false
 */
int gdTransformAffineGetImage(gdImagePtr *dst,
          const gdImagePtr src,
          gdRectPtr src_area,
          const double affine[6])
{
    int res;
    double m[6];
    gdRect bbox;
    gdRect area_full;

    if (src_area == NULL) {
        area_full.x = 0;
        area_full.y = 0;
        area_full.width  = gdImageSX(src);
        area_full.height = gdImageSY(src);
        src_area = &area_full;
    }

    gdTransformAffineBoundingBox(src_area, affine, &bbox);

    *dst = gdImageCreateTrueColor(bbox.width, bbox.height);
    if (*dst == NULL) {
        return false;
    }
    (*dst)->saveAlphaFlag = 1;

    //if (!src->trueColor) {
    //    gdImagePaletteToTrueColor(src);
    //}

    // Translate to dst origin (0,0)
    gdAffineTranslate(m, -bbox.x, -bbox.y);
    gdAffineConcat(m, affine, m);

    gdImageAlphaBlending(*dst, 0);

    res = gdTransformAffineCopy(*dst,
          0,0,
          src,
          src_area,
          m);

    if (res != true) {
        gdImageDestroy(*dst);
        *dst = NULL;
        return false;
    } else {
        return true;
    }
}



/**
 * Function: gdTransformAffineCopy
 *  Applies an affine transformation to a region and copy the result
 *  in a destination to the given position.
 *
 * Parameters:
 * 	dst - Image to draw the transformed image
 *  src - Source image
 *  dstX - X position in the destination image
 *  dstY - Y position in the destination image
 *  src_area - Rectangular region to rotate in the src image
 *
 * Returns:
 *  true on success or false on failure
 */
int gdTransformAffineCopy(gdImagePtr dst,
          int dst_x, int dst_y,
          const gdImagePtr src,
          gdRectPtr src_region,
          const double affine[6])
{
    int c1x,c1y,c2x,c2y;
    int backclip = 0;
    int backup_clipx1, backup_clipy1, backup_clipx2, backup_clipy2;
    register int x, y, src_offset_x, src_offset_y;
    double inv[6];
    gdPointF pt, src_pt;
    gdRect bbox;
    int end_x, end_y;
    gdInterpolationMethod interpolation_id_bak = src->interpolation_id;

    // These methods use special implementations
    if (src->interpolation_id == GD_BILINEAR_FIXED || src->interpolation_id == GD_BICUBIC_FIXED || src->interpolation_id == GD_NEAREST_NEIGHBOUR) {
        gdImageSetInterpolationMethod(src, GD_BICUBIC);
    }

    gdImageClipRectangle(src, src_region);
    c1x = src_region->x;
    c1y = src_region->y;
    c2x = src_region->x + src_region->width -1;
    c2y = src_region->y + src_region->height -1;

    if (src_region->x > 0 || src_region->y > 0
        || src_region->width < gdImageSX(src)
        || src_region->height < gdImageSY(src)) {
        backclip = 1;

        gdImageGetClip(src, &backup_clipx1, &backup_clipy1,
        &backup_clipx2, &backup_clipy2);

        gdImageSetClip(src, src_region->x, src_region->y,
            src_region->x + src_region->width - 1,
            src_region->y + src_region->height - 1);
    }

    if (!gdTransformAffineBoundingBox(src_region, affine, &bbox)) {
        if (backclip) {
            gdImageSetClip(src, backup_clipx1, backup_clipy1,
                    backup_clipx2, backup_clipy2);
        }
        gdImageSetInterpolationMethod(src, interpolation_id_bak);
        return false;
    }

    end_x = bbox.width  + abs(bbox.x);
    end_y = bbox.height + abs(bbox.y);

    // Get inverse affine to let us work with destination -> source
    if (gdAffineInvert(inv, affine) == false) {
        gdImageSetInterpolationMethod(src, interpolation_id_bak);
        return false;
    }

    src_offset_x =  src_region->x;
    src_offset_y =  src_region->y;

    //if (dst->alphaBlendingFlag)
    //{
    //    for (y = bbox.y; y <= end_y; y++) {
    //        pt.y = y + 0.5;
    //        for (x = bbox.x; x <= end_x; x++) {
    //            pt.x = x + 0.5;
    //            gdAffineApplyToPointF(&src_pt, &pt, inv);
    //            if (floor(src_offset_x + src_pt.x) < c1x
    //                || floor(src_offset_x + src_pt.x) > c2x
    //                || floor(src_offset_y + src_pt.y) < c1y
    //                || floor(src_offset_y + src_pt.y) > c2y) {
    //                continue;
    //            }
    //            gdImageSetPixel(dst, dst_x + x, dst_y + y, getPixelInterpolated(src, (int)(src_offset_x + src_pt.x), (int)(src_offset_y + src_pt.y), 0));
    //        }
    //    }
    //}
    //else
    {
        for (y = bbox.y; y <= end_y; y++) {
            //unsigned char *dst_p = NULL;
            int *tdst_p = NULL;

            pt.y = y + 0.5;
            if ((dst_y + y) < 0 || ((dst_y + y) > gdImageSY(dst) -1)) {
                continue;
            }
            tdst_p = dst->tpixels[dst_y + y] + dst_x;

            for (x = bbox.x; x <= end_x; x++) {
                pt.x = x + 0.5;
                gdAffineApplyToPointF(&src_pt, &pt, inv);

                if ((dst_x + x) < 0 || (dst_x + x) > (gdImageSX(dst) - 1)) {
                    break;
                }
                if (floor(src_offset_x + src_pt.x) < c1x
                    || floor(src_offset_x + src_pt.x) > c2x
                    || floor(src_offset_y + src_pt.y) < c1y
                    || floor(src_offset_y + src_pt.y) > c2y) {
                    continue;
                }
                *(tdst_p + dst_x + x) = getPixelInterpolated(src, (int)(src_offset_x + src_pt.x), (int)(src_offset_y + src_pt.y), -1);
            }
        }
    }

    // Restore clip if required
    if (backclip) {
        gdImageSetClip(src, backup_clipx1, backup_clipy1,
                backup_clipx2, backup_clipy2);
    }

    gdImageSetInterpolationMethod(src, interpolation_id_bak);
    return true;
}

int gdTransformAffineBoundingBox(gdRectPtr src, const double affine[6], gdRectPtr bbox);

/**
 * Function: gdTransformAffineBoundingBox
 *  Returns the bounding box of an affine transformation applied to a
 *  rectangular area <gdRect>
 *
 * Parameters:
 * 	src - Rectangular source area for the affine transformation
 *  affine - the affine transformation
 *  bbox - the resulting bounding box
 *
 * Returns:
 *  true if the affine is rectilinear or false
 */
int gdTransformAffineBoundingBox(gdRectPtr src, const double affine[6], gdRectPtr bbox)
{
    gdPointF extent[4], min, max, point;
    int i;

    extent[0].x=0.0;
    extent[0].y=0.0;
    extent[1].x=(double) src->width;
    extent[1].y=0.0;
    extent[2].x=(double) src->width;
    extent[2].y=(double) src->height;
    extent[3].x=0.0;
    extent[3].y=(double) src->height;

    for (i=0; i < 4; i++) {
        point=extent[i];
        if (gdAffineApplyToPointF(&extent[i], &point, affine) != true) {
            return false;
        }
    }
    min=extent[0];
    max=extent[0];

    for (i=1; i < 4; i++) {
        if (min.x > extent[i].x)
            min.x=extent[i].x;
        if (min.y > extent[i].y)
            min.y=extent[i].y;
        if (max.x < extent[i].x)
            max.x=extent[i].x;
        if (max.y < extent[i].y)
            max.y=extent[i].y;
    }
    bbox->x = (int) min.x;
    bbox->y = (int) min.y;
    bbox->width  = (int) ceil((max.x - min.x));
    bbox->height = (int) ceil(max.y - min.y);

    return true;
}


/**
 * InternalFunction: getPixelInterpolated
 *  Returns the interpolated color value using the default interpolation
 *  method. The returned color is always in the ARGB format (truecolor).
 *
 * Parameters:
 * 	im - Image to set the default interpolation method
 *  y - X value of the ideal position
 *  y - Y value of the ideal position
 *  method - Interpolation method <gdInterpolationMethod>
 *
 * Returns:
 *  the interpolated color or -1 on error
 *
 * See also:
 *  <gdSetInterpolationMethod>
 */
static int getPixelInterpolated(gdImagePtr im, const double x, const double y, const int bgColor)
{
    const int xi=(int)(x);
    const int yi=(int)(y);
    int yii;
    int i;
    double kernel, kernel_cache_y;
    double kernel_x[12], kernel_y[4];
    double new_r = 0.0f, new_g = 0.0f, new_b = 0.0f, new_a = 0.0f;

    // These methods use special implementations
    if (im->interpolation_id == GD_NEAREST_NEIGHBOUR) {
        return -1;
    }

    if (im->interpolation_id == GD_WEIGHTED4) {
        return getPixelInterpolateWeight(im, x, y, bgColor);
    }

    if (im->interpolation) {
        for (i=0; i<4; i++) {
            kernel_x[i] = (double) im->interpolation((double)(xi+i-1-x), 1.0);
            kernel_y[i] = (double) im->interpolation((double)(yi+i-1-y), 1.0);
        }
    } else {
        return -1;
    }

    /*
     * TODO: use the known fast rgba multiplication implementation once
     * the new formats are in place
     */
    for (yii = yi-1; yii < yi+3; yii++) {
        int xii;
        kernel_cache_y = kernel_y[yii-(yi-1)];
        if (im->trueColor) {
            for (xii=xi-1; xii<xi+3; xii++) {
                const int rgbs = getPixelOverflowTC(im, xii, yii, bgColor);

                kernel = kernel_cache_y * kernel_x[xii-(xi-1)];
                new_r += kernel * gdTrueColorGetRed(rgbs);
                new_g += kernel * gdTrueColorGetGreen(rgbs);
                new_b += kernel * gdTrueColorGetBlue(rgbs);
                new_a += kernel * gdTrueColorGetAlpha(rgbs);
            }
        }
        //else {
        //    for (xii=xi-1; xii<xi+3; xii++) {
        //        const int rgbs = getPixelOverflowPalette(im, xii, yii, bgColor);

        //        kernel = kernel_cache_y * kernel_x[xii-(xi-1)];
        //        new_r += kernel * gdTrueColorGetRed(rgbs);
        //        new_g += kernel * gdTrueColorGetGreen(rgbs);
        //        new_b += kernel * gdTrueColorGetBlue(rgbs);
        //        new_a += kernel * gdTrueColorGetAlpha(rgbs);
        //    }
        //}
    }

    new_r = CLAMP(new_r, 0, 255);
    new_g = CLAMP(new_g, 0, 255);
    new_b = CLAMP(new_b, 0, 255);
    new_a = CLAMP(new_a, 0, 255);

    return gdTrueColorAlpha(((int)new_r), ((int)new_g), ((int)new_b), ((int)new_a));
}

static int getPixelInterpolateWeight(gdImagePtr im, const double x, const double y, const int bgColor)
{
    // Closest pixel <= (xf,yf)
    int sx = (int)(x);
    int sy = (int)(y);
    const double xf = x - (double)sx;
    const double yf = y - (double)sy;
    const double nxf = (double) 1.0 - xf;
    const double nyf = (double) 1.0 - yf;
    const double m1 = xf * yf;
    const double m2 = nxf * yf;
    const double m3 = xf * nyf;
    const double m4 = nxf * nyf;

    // get color values of neighbouring pixels
    const int c1 = getPixelOverflowTC(im, sx, sy, bgColor);
    const int c2 = getPixelOverflowTC(im, sx - 1, sy, bgColor);
    const int c3 = getPixelOverflowTC(im, sx, sy - 1, bgColor);
    const int c4 = getPixelOverflowTC(im, sx - 1, sy - 1, bgColor);
    int r, g, b, a;

    if (x < 0) sx--;
    if (y < 0) sy--;

    // component-wise summing-up of color values
    r = (int)(m1*gdTrueColorGetRed(c1)   + m2*gdTrueColorGetRed(c2)   + m3*gdTrueColorGetRed(c3)   + m4*gdTrueColorGetRed(c4));
    g = (int)(m1*gdTrueColorGetGreen(c1) + m2*gdTrueColorGetGreen(c2) + m3*gdTrueColorGetGreen(c3) + m4*gdTrueColorGetGreen(c4));
    b = (int)(m1*gdTrueColorGetBlue(c1)  + m2*gdTrueColorGetBlue(c2)  + m3*gdTrueColorGetBlue(c3)  + m4*gdTrueColorGetBlue(c4));
    a = (int)(m1*gdTrueColorGetAlpha(c1) + m2*gdTrueColorGetAlpha(c2) + m3*gdTrueColorGetAlpha(c3) + m4*gdTrueColorGetAlpha(c4));

    r = CLAMP(r, 0, 255);
    g = CLAMP(g, 0, 255);
    b = CLAMP(b, 0, 255);
    a = CLAMP(a, 0, 255);
    return gdTrueColorAlpha(r, g, b, a);
}

static inline int getPixelOverflowColorTC(gdImagePtr im, const int x, const int y, const int color)
{
    (void) color;

    if (gdImageBoundsSafe(im, x, y)) {
        const int c = im->tpixels[y][x];
        if (c == im->transparent) {
            return gdTrueColorAlpha(0, 0, 0, 127);
        }
        return c;
    } else {
        register int border = 0;
        if (y < im->cy1) {
            border = im->tpixels[0][im->cx1];
            goto processborder;
        }

        if (y < im->cy1) {
            border = im->tpixels[0][im->cx1];
            goto processborder;
        }

        if (y > im->cy2) {
            if (x >= im->cx1 && x <= im->cx1) {
                border = im->tpixels[im->cy2][x];
                goto processborder;
            } else {
                return gdTrueColorAlpha(0, 0, 0, 127);
            }
        }

        // y is bound safe at this point
        if (x < im->cx1) {
            border = im->tpixels[y][im->cx1];
            goto processborder;
        }

        if (x > im->cx2) {
            border = im->tpixels[y][im->cx2];
        }

processborder:
        if (border == im->transparent) {
            return gdTrueColorAlpha(0, 0, 0, 127);
        } else{
            return gdTrueColorAlpha(gdTrueColorGetRed(border), gdTrueColorGetGreen(border), gdTrueColorGetBlue(border), 127);
        }
    }
}



#endif


