/*
 * The two pass scaling function is based on:
 * Filtered Image Rescaling
 * Based on Gems III
 *  - Schumacher general filtered image rescaling
 * (pp. 414-424)
 * by Dale Schumacher
 *
 * 	Additional changes by Ray Gardener, Daylon Graphics Ltd.
 * 	December 4, 1999
 *
 * 	Ported to libgd by Pierre Joye. Support for multiple channels
 * 	added (argb for now).
 *
 * 	Initial sources code is avaibable in the Gems Source Code Packages:
 * 	http://www.acm.org/pubs/tog/GraphicsGems/GGemsIII.tar.gz
 *
 */

#include "config.h"
#include "gd-resize.h"

#include "gd-helpers.h"
#include <inttypes.h>
#include <math.h>
#include <assert.h>

static gdImage* _gd_img_scale_nearest_neighbour(gdImage *im,
                                                const unsigned int width,
                                                const unsigned int height);
static gdImage* _gd_img_scale_bilinear(gdImage *im,
                                       const unsigned int new_width,
                                       const unsigned int new_height);
static gdImage* _gd_img_scale_bicubic_fixed(gdImage *src,
                                            const unsigned int width,
                                            const unsigned int height);
static gdImage* _gd_img_scale_two_pass(gdImage *src,
                                       const unsigned int new_width,
                                       const unsigned int new_height);

/*
        Summary:

                - Horizontal filter contributions are calculated on the fly,
                  as each column is mapped from src to dst image. This lets
                  us omit having to allocate a temporary full horizontal stretch
                  of the src image.

                - If none of the src pixels within a sampling region differ,
                  then the output pixel is forced to equal (any of) the source pixel.
                  This ensures that filters do not corrupt areas of constant color.

                - Filter weight contribution results, after summing, are
                  rounded to the nearest pixel color value instead of
                  being casted to ILubyte (usually an int or char). Otherwise,
                  artifacting occurs.
*/

/*
        Additional functions are available for simple rotation or up/downscaling.
        downscaling using the fixed point implementations are usually much faster
        than the existing gdImageCopyResampled while having a similar or better
        quality.

        For image rotations, the optimized versions have a lazy antialiasing for
        the edges of the images. For a much better antialiased result, the affine
        function is recommended.
*/

/*
TODO:
 - Optimize pixel accesses and loops once we have continuous buffer
 - Add scale support for a portion only of an image (equivalent of copyresized/resampled)
 */

/* only used here, let do a generic fixed point integers later if required by other
   part of GD */

typedef long gdFixed;

// Integer to fixed point
#define gd_itofx(x) ((x) << 8)

// Float to fixed point
#define gd_ftofx(x) (long)((x)*256)

//  Double to fixed point
#define gd_dtofx(x) (long)((x)*256)

// Fixed point to integer
#define gd_fxtoi(x) ((x) >> 8)

// Fixed point to float
#define gd_fxtof(x) ((float)(x) / 256)

// Fixed point to double
#define gd_fxtod(x) ((double)(x) / 256)

// Multiply a fixed by a fixed
#define gd_mulfx(x, y) (((x) * (y)) >> 8)

// Divide a fixed by a fixed
#define gd_divfx(x, y) (((x) << 8) / (y))

typedef struct _FilterInfo
{
    double (*function)(const double, const double), support;

} FilterInfo;

typedef struct
{
    double *Weights; // Normalized weights of neighboring pixels
    int Left, Right; // Bounds of source pixels window

} ContributionType; // Contirbution information for a single pixel

typedef struct
{
    ContributionType *ContribRow; // Row (or column) of contribution weights
    unsigned int WindowSize,      // Filter window size (of affecting source pixels)
        LineLength;               // Length of line (no. or rows / cols)
} LineContribType;

gdImage *gd_img_scale(gdImage *src,
                      unsigned int new_width, unsigned int new_height)
{
    gdImage *im_scaled = NULL;

    if (src == NULL
        || new_width > VNR_MAX_SIZE || new_height > VNR_MAX_SIZE
        || (uintmax_t) src->interpolation_id >= GD_METHOD_COUNT)
    {
        return NULL;
    }

    if (new_width == 0 || new_height == 0)
        return NULL;

    if ((int) new_width == gd_img_sx(src)
        && (int) new_height == gd_img_sy(src))
    {
        return gd_img_copy(src);
    }

    switch (src->interpolation_id)
    {
    case GD_NEAREST_NEIGHBOUR:
        im_scaled = _gd_img_scale_nearest_neighbour(
                                        src, new_width, new_height);
        break;

    case GD_BILINEAR_FIXED:
    case GD_LINEAR:
        im_scaled = _gd_img_scale_bilinear(src, new_width, new_height);
        break;

    case GD_BICUBIC_FIXED:
    case GD_BICUBIC:
        im_scaled = _gd_img_scale_bicubic_fixed(src, new_width, new_height);
        break;

    default:
        if (src->interpolation == NULL)
            return NULL;

        im_scaled = _gd_img_scale_two_pass(src, new_width, new_height);
        break;
    }

    return im_scaled;
}

static double _kernel_bessel_j1(const double x)
{
    double p, q;

    register long i;

    static const double
        Pone[] =
            {
                0.581199354001606143928050809e+21,
                -0.6672106568924916298020941484e+20,
                0.2316433580634002297931815435e+19,
                -0.3588817569910106050743641413e+17,
                0.2908795263834775409737601689e+15,
                -0.1322983480332126453125473247e+13,
                0.3413234182301700539091292655e+10,
                -0.4695753530642995859767162166e+7,
                0.270112271089232341485679099e+4},
        Qone[] =
            {
                0.11623987080032122878585294e+22,
                0.1185770712190320999837113348e+20,
                0.6092061398917521746105196863e+17,
                0.2081661221307607351240184229e+15,
                0.5243710262167649715406728642e+12,
                0.1013863514358673989967045588e+10,
                0.1501793594998585505921097578e+7,
                0.1606931573481487801970916749e+4,
                0.1e+1};

    p = Pone[8];
    q = Qone[8];
    for (i = 7; i >= 0; i--)
    {
        p = p * x * x + Pone[i];
        q = q * x * x + Qone[i];
    }
    return (double)(p / q);
}

static double _kernel_bessel_p1(const double x)
{
    double p, q;

    register long i;

    static const double
        Pone[] =
            {
                0.352246649133679798341724373e+5,
                0.62758845247161281269005675e+5,
                0.313539631109159574238669888e+5,
                0.49854832060594338434500455e+4,
                0.2111529182853962382105718e+3,
                0.12571716929145341558495e+1},
        Qone[] =
            {
                0.352246649133679798068390431e+5,
                0.626943469593560511888833731e+5,
                0.312404063819041039923015703e+5,
                0.4930396490181088979386097e+4,
                0.2030775189134759322293574e+3,
                0.1e+1};

    p = Pone[5];
    q = Qone[5];
    for (i = 4; i >= 0; i--)
    {
        p = p * (8.0 / x) * (8.0 / x) + Pone[i];
        q = q * (8.0 / x) * (8.0 / x) + Qone[i];
    }
    return (double)(p / q);
}

static double _kernel_bessel_q1(const double x)
{
    double p, q;

    register long i;

    static const double
        Pone[] =
            {
                0.3511751914303552822533318e+3,
                0.7210391804904475039280863e+3,
                0.4259873011654442389886993e+3,
                0.831898957673850827325226e+2,
                0.45681716295512267064405e+1,
                0.3532840052740123642735e-1},
        Qone[] =
            {
                0.74917374171809127714519505e+4,
                0.154141773392650970499848051e+5,
                0.91522317015169922705904727e+4,
                0.18111867005523513506724158e+4,
                0.1038187585462133728776636e+3,
                0.1e+1};

    p = Pone[5];
    q = Qone[5];
    for (i = 4; i >= 0; i--)
    {
        p = p * (8.0 / x) * (8.0 / x) + Pone[i];
        q = q * (8.0 / x) * (8.0 / x) + Qone[i];
    }
    return (double)(p / q);
}

static double _kernel_bessel_order1(double x)
{
    double p, q;

    if (x == 0.0)
        return (0.0f);
    p = x;
    if (x < 0.0)
        x = (-x);
    if (x < 8.0)
        return (p * _kernel_bessel_j1(x));
    q = (double)sqrt(2.0f / (M_PI * x)) * (double)(_kernel_bessel_p1(x) * (1.0f / sqrt(2.0f) * (sin(x) - cos(x))) - 8.0f / x * _kernel_bessel_q1(x) *
                                                                                                                      (-1.0f / sqrt(2.0f) * (sin(x) + cos(x))));
    if (p < 0.0f)
        q = (-q);
    return (q);
}

static double _filter_sinc(const double x, const double support)
{
    (void)support;

    // X-scaled Sinc(x) function.
    if (x == 0.0)
        return (1.0);
    return (sin(M_PI * (double)x) / (M_PI * (double)x));
}

static double _filter_bessel(const double x, const double support)
{
    (void)support;
    if (x == 0.0f)
        return (double)(M_PI / 4.0f);
    return (_kernel_bessel_order1((double)M_PI * x) / (2.0f * x));
}

static double _filter_blackman(const double x, const double support)
{
    (void)support;
    return (0.42f + 0.5f * (double)cos(M_PI * x) + 0.08f * (double)cos(2.0f * M_PI * x));
}

static double _filter_linear(const double x, const double support)
{
    (void) support;
    double ax = fabs(x);

    if (ax < 1.0f)
        return (1.0f - ax);

    return 0.0f;
}

static double _filter_blackman_bessel(const double x, const double support)
{
    (void) support;
    return (_filter_blackman(x / support, support) * _filter_bessel(x, support));
}

static double _filter_blackman_sinc(const double x, const double support)
{
    (void) support;
    return (_filter_blackman(x / support, support) * _filter_sinc(x, support));
}

/**
 * Generalized cubic kernel (for a=-1 it is the same as BicubicKernel):
  \verbatim
          /
         | (a+2)|t|**3 - (a+3)|t|**2 + 1     , |t| <= 1
  h(t) = | a|t|**3 - 5a|t|**2 + 8a|t| - 4a   , 1 < |t| <= 2
         | 0                                 , otherwise
          \
  \endverbatim
 * Often used values for a are -1 and -1/2.
 */

static double _filter_generalized_cubic(const double t, const double support)
{
    const double a = -support;
    double abs_t = (double)fabs(t);
    double abs_t_sq = abs_t * abs_t;
    if (abs_t < 1)
        return (a + 2) * abs_t_sq * abs_t - (a + 3) * abs_t_sq + 1;
    if (abs_t < 2)
        return a * abs_t_sq * abs_t - 5 * a * abs_t_sq + 8 * a * abs_t - 4 * a;
    return 0;
}

// CubicSpline filter, default radius 2
static double _filter_cubic_spline(const double x1, const double support)
{
    (void)support;
    const double x = x1 < 0.0 ? -x1 : x1;

    if (x < 1.0)
    {
        const double x2 = x * x;

        return (0.5 * x2 * x - x2 + 2.0 / 3.0);
    }
    if (x < 2.0)
    {
        return (pow(2.0 - x, 3.0) / 6.0);
    }
    return 0;
}

static double _filter_box(double x, const double support)
{
    if (x < -support)
        return 0.0f;
    if (x < support)
        return 1.0f;
    return 0.0f;
}

static double _filter_catmullrom(const double x, const double support)
{
    (void)support;
    if (x < -2.0)
        return (0.0f);
    if (x < -1.0)
        return (0.5f * (4.0f + x * (8.0f + x * (5.0f + x))));
    if (x < 0.0)
        return (0.5f * (2.0f + x * x * (-5.0f - 3.0f * x)));
    if (x < 1.0)
        return (0.5f * (2.0f + x * x * (-5.0f + 3.0f * x)));
    if (x < 2.0)
        return (0.5f * (4.0f + x * (-8.0f + x * (5.0f - x))));
    return (0.0f);
}

// Lanczos8 filter, default radius 8
static double _filter_lanczos8(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;

    if (x == 0.0)
        return 1;

    if (x < support)
    {
        return support * sin(x * M_PI) * sin(x * M_PI / support) / (x * M_PI * x * M_PI);
    }
    return 0.0;
}

static double filter_lanczos3(const double x1, const double support)
{
    if (x1 < -3.0)
        return (0.0);
    if (x1 < 0.0)
        return (_filter_sinc(-x1, support) * _filter_sinc(-x1 / 3.0, support));
    if (x1 < 3.0)
        return (_filter_sinc(x1, support) * _filter_sinc(x1 / 3.0, support));
    return (0.0);
}

// Hermite filter, default radius 1
static double filter_hermite(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    (void)support;

    if (x < 1.0)
        return ((2.0 * x - 3) * x * x + 1.0);

    return 0.0;
}

// Trangle filter, default radius 1
static double filter_triangle(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    (void)support;

    if (x < 1.0)
        return (1.0 - x);
    return 0.0;
}

// Bell filter, default radius 1.5
static double filter_bell(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    (void)support;

    if (x < 0.5)
        return (0.75 - x * x);
    if (x < 1.5)
        return (0.5 * pow(x - 1.5, 2.0));
    return 0.0;
}

// Mitchell filter, default radius 2.0
static double filter_mitchell(const double x, const double support)
{
    (void)support;
#define KM_B (1.0f / 3.0f)
#define KM_C (1.0f / 3.0f)
#define KM_P0 ((6.0f - 2.0f * KM_B) / 6.0f)
#define KM_P2 ((-18.0f + 12.0f * KM_B + 6.0f * KM_C) / 6.0f)
#define KM_P3 ((12.0f - 9.0f * KM_B - 6.0f * KM_C) / 6.0f)
#define KM_Q0 ((8.0f * KM_B + 24.0f * KM_C) / 6.0f)
#define KM_Q1 ((-12.0f * KM_B - 48.0f * KM_C) / 6.0f)
#define KM_Q2 ((6.0f * KM_B + 30.0f * KM_C) / 6.0f)
#define KM_Q3 ((-1.0f * KM_B - 6.0f * KM_C) / 6.0f)

    if (x < -2.0)
        return (0.0f);
    if (x < -1.0)
        return (KM_Q0 - x * (KM_Q1 - x * (KM_Q2 - x * KM_Q3)));
    if (x < 0.0f)
        return (KM_P0 + x * x * (KM_P2 - x * KM_P3));
    if (x < 1.0f)
        return (KM_P0 + x * x * (KM_P2 + x * KM_P3));
    if (x < 2.0f)
        return (KM_Q0 + x * (KM_Q1 + x * (KM_Q2 + x * KM_Q3)));

    return (0.0f);
}

// Cosine filter, default radius 1
static double filter_cosine(const double x, const double support)
{
    (void)support;
    if ((x >= -1.0) && (x <= 1.0))
        return ((cos(x * M_PI) + 1.0) / 2.0);

    return 0;
}

// Quadratic filter, default radius 1.5
static double filter_quadratic(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    (void)support;
    if (x <= 0.5)
        return (-2.0 * x * x + 1);
    if (x <= 1.5)
        return (x * x - 2.5 * x + 1.5);
    return 0.0;
}

static double filter_bspline(const double x, const double support)
{
    (void)support;
    if (x > 2.0f)
    {
        return 0.0f;
    }
    else
    {
        double a, b, c, d;
        // Was calculated anyway cause the "if((x-1.0f) < 0)"
        const double xm1 = x - 1.0f;
        const double xp1 = x + 1.0f;
        const double xp2 = x + 2.0f;

        if ((xp2) <= 0.0f)
            a = 0.0f;
        else
            a = xp2 * xp2 * xp2;
        if ((xp1) <= 0.0f)
            b = 0.0f;
        else
            b = xp1 * xp1 * xp1;
        if (x <= 0)
            c = 0.0f;
        else
            c = x * x * x;
        if ((xm1) <= 0.0f)
            d = 0.0f;
        else
            d = xm1 * xm1 * xm1;

        return (0.16666666666666666667f * (a - (4.0f * b) + (6.0f * c) - (4.0f * d)));
    }
}

// QuadraticBSpline filter, default radius 1.5
static double filter_quadratic_bspline(const double x1, const double support)
{
    const double x = x1 < 0.0 ? -x1 : x1;
    (void)support;
    if (x <= 0.5)
        return (-x * x + 0.75);
    if (x <= 1.5)
        return (0.5 * x * x - 1.5 * x + 1.125);
    return 0.0;
}

static double filter_gaussian(const double x, const double support)
{
    (void)support;
    // return(exp((double) (-2.0 * x * x)) * sqrt(2.0 / M_PI));
    return (double)(exp(-2.0f * x * x) * 0.79788456080287f);
}

static double filter_hanning(const double x, const double support)
{
    (void)support;
    // A Cosine windowing function
    return (0.5 + 0.5 * cos(M_PI * x));
}

static double filter_hamming(const double x, const double support)
{
    (void)support;
    /* should be
    (0.54+0.46*cos(M_PI*(double) x));
    but this approximation is sufficient */
    if (x < -1.0f)
        return 0.0f;
    if (x < 0.0f)
        return 0.92f * (-2.0f * x - 3.0f) * x * x + 1.0f;
    if (x < 1.0f)
        return 0.92f * (2.0f * x - 3.0f) * x * x + 1.0f;
    return 0.0f;
}

static double filter_power(const double x, const double support)
{
    (void)support;
    const double a = 2.0f;
    if (fabs(x) > 1)
        return 0.0f;
    return (1.0f - (double)fabs(pow(x, a)));
}

static double filter_welsh(const double x, const double support)
{
    (void)support;
    // Welsh parabolic windowing filter
    if (x < 1.0)
        return (1 - x * x);
    return (0.0);
}

static inline int getPixelOverflowTC(gdImage *im,
                                     const int x, const int y,
                                     const int bgColor)
{
    if (gd_img_bounds_safe_macro(im, x, y))
    {
        const uint32_t c = im->tpixels[y][x];

        if (c == im->transparent)
        {
            return bgColor == -1 ? gd_set_alpha(0, 0, 0, 255) : bgColor;
        }

        return c; // 31bit ARGB TC
    }
    else
    {
        return bgColor; // 31bit ARGB TC
    }
}

static inline LineContribType *_gdContributionsAlloc(unsigned int line_length, unsigned int windows_size)
{
    unsigned int u = 0;
    LineContribType *res;
    size_t weights_size;

    if (overflow2(windows_size, sizeof(double)))
    {
        return NULL;
    }
    else
    {
        weights_size = windows_size * sizeof(double);
    }
    res = (LineContribType *)malloc(sizeof(LineContribType));
    if (!res)
    {
        return NULL;
    }
    res->WindowSize = windows_size;
    res->LineLength = line_length;
    if (overflow2(line_length, sizeof(ContributionType)))
    {
        free(res);
        return NULL;
    }
    res->ContribRow = (ContributionType *)malloc(line_length * sizeof(ContributionType));
    if (res->ContribRow == NULL)
    {
        free(res);
        return NULL;
    }
    for (u = 0; u < line_length; u++)
    {
        res->ContribRow[u].Weights = (double *)malloc(weights_size);
        if (res->ContribRow[u].Weights == NULL)
        {
            unsigned int i;

            for (i = 0; i < u; i++)
            {
                free(res->ContribRow[i].Weights);
            }
            free(res->ContribRow);
            free(res);
            return NULL;
        }
    }
    return res;
}

static inline void _gdContributionsFree(LineContribType *p)
{
    unsigned int u;
    for (u = 0; u < p->LineLength; u++)
    {
        free(p->ContribRow[u].Weights);
    }
    free(p->ContribRow);
    free(p);
}

static inline LineContribType *_gdContributionsCalc(unsigned int line_size, unsigned int src_size, double scale_d, const double support, const interpolation_method pFilter)
{
    double width_d;
    double scale_f_d = 1.0;
    const double filter_width_d = support;
    int windows_size;
    unsigned int u;
    LineContribType *res;

    if (scale_d < 1.0)
    {
        width_d = filter_width_d / scale_d;
        scale_f_d = scale_d;
    }
    else
    {
        width_d = filter_width_d;
    }

    windows_size = 2 * (int)ceil(width_d) + 1;
    res = _gdContributionsAlloc(line_size, windows_size);
    if (res == NULL)
    {
        return NULL;
    }
    for (u = 0; u < line_size; u++)
    {
        const double dCenter = (double)u / scale_d;
        // get the significant edge points affecting the pixel
        register int iLeft = MAX(0, (int)floor(dCenter - width_d));
        int iRight = MIN((int)ceil(dCenter + width_d), (int)src_size - 1);
        double dTotalWeight = 0.0;
        int iSrc;

        // Cut edge points to fit in filter window in case of spill-off
        if (iRight - iLeft + 1 > windows_size)
        {
            if (iLeft < ((int)src_size - 1 / 2))
            {
                iLeft++;
            }
            else
            {
                iRight--;
            }
        }

        res->ContribRow[u].Left = iLeft;
        res->ContribRow[u].Right = iRight;

        for (iSrc = iLeft; iSrc <= iRight; iSrc++)
        {
            dTotalWeight += (res->ContribRow[u].Weights[iSrc - iLeft] = scale_f_d * (*pFilter)(scale_f_d * (dCenter - (double)iSrc), support));
        }

        if (dTotalWeight < 0.0)
        {
            _gdContributionsFree(res);
            return NULL;
        }

        if (dTotalWeight > 0.0)
        {
            for (iSrc = iLeft; iSrc <= iRight; iSrc++)
            {
                res->ContribRow[u].Weights[iSrc - iLeft] /= dTotalWeight;
            }
        }
    }
    return res;
}

static inline void _gdScaleOneAxis(const gdImage *pSrc, gdImage *dst,
                                   unsigned int dst_len, unsigned int row,
                                   LineContribType *contrib,
                                   gdAxis axis)
{
    unsigned int ndx;

    for (ndx = 0; ndx < dst_len; ndx++)
    {
        double r = 0, g = 0, b = 0, a = 0;
        const int left = contrib->ContribRow[ndx].Left;
        const int right = contrib->ContribRow[ndx].Right;
        uint32_t *dest = (axis == HORIZONTAL) ? &dst->tpixels[row][ndx] : &dst->tpixels[ndx][row];

        uint32_t i;

        // Accumulate each channel
        for (i = left; i <= right; i++)
        {
            const uint32_t left_channel = i - left;
            const uint32_t srcpx = (axis == HORIZONTAL) ? pSrc->tpixels[row][i] : pSrc->tpixels[i][row];

            r += contrib->ContribRow[ndx].Weights[left_channel] * (double)(gd_get_red(srcpx));
            g += contrib->ContribRow[ndx].Weights[left_channel] * (double)(gd_get_green(srcpx));
            b += contrib->ContribRow[ndx].Weights[left_channel] * (double)(gd_get_blue(srcpx));
            a += contrib->ContribRow[ndx].Weights[left_channel] * (double)(gd_get_alpha(srcpx));
        } // for

        *dest = gd_set_alpha(uchar_clamp(r, 0xFF), uchar_clamp(g, 0xFF),
                             uchar_clamp(b, 0xFF),
                             uchar_clamp(a, 0xFF));
    } // for
} // _gdScaleOneAxis

static inline int _gdScalePass(const gdImage *pSrc, const unsigned int src_len,
                               gdImage *pDst, const unsigned int dst_len,
                               const unsigned int num_lines,
                               const gdAxis axis,
                               const FilterInfo *filter)
{
    unsigned int line_ndx;
    LineContribType *contrib;

    // Same dim, just copy it.
    assert(dst_len != src_len); // TODO: caller should handle this.

    contrib = _gdContributionsCalc(dst_len, src_len,
                                   (double)dst_len / (double)src_len,
                                   filter->support,
                                   filter->function);
    if (contrib == NULL)
    {
        return 0;
    }

    // Scale each line
    for (line_ndx = 0; line_ndx < num_lines; line_ndx++)
    {
        _gdScaleOneAxis(pSrc, pDst, dst_len, line_ndx, contrib, axis);
    }
    _gdContributionsFree(contrib);
    return 1;
} // _gdScalePass

static const FilterInfo filters[GD_METHOD_COUNT + 1] =
    {
        {_filter_box, 0.0},
        {filter_bell, 1.5},
        {_filter_bessel, 0.0},
        {NULL, 0.0}, // NA bilenear/bilinear fixed
        {NULL, 0.0}, // NA bicubic
        {NULL, 0.0}, // NA bicubic fixed
        {_filter_blackman, 1.0},
        {_filter_box, 0.5},
        {filter_bspline, 1.5},
        {_filter_catmullrom, 2.0},
        {filter_gaussian, 1.25},
        {_filter_generalized_cubic, 0.5},
        {filter_hermite, 1.0},
        {filter_hamming, 1.0},
        {filter_hanning, 1.0},
        {filter_mitchell, 2.0},
        {NULL, 0.0}, // NA Nearest
        {filter_power, 0.0},
        {filter_quadratic, 1.5},
        {_filter_sinc, 1.0},
        {filter_triangle, 1.0},
        {NULL, 1.0}, // NA weighted4
        {_filter_linear, 1.0},
        {filter_lanczos3, 3.0},
        {_filter_lanczos8, 8.0},
        {_filter_blackman_bessel, 3.2383},
        {_filter_blackman_sinc, 4.0},
        {filter_quadratic_bspline, 1.5},
        {_filter_cubic_spline, 0.0},
        {filter_cosine, 0.0},
        {filter_welsh, 0.0},
};

static const FilterInfo *_get_filterinfo_for_id(gdInterpolationMethod id)
{

    if (id >= GD_METHOD_COUNT)
    {
        id = GD_DEFAULT;
    }
    return &filters[id];
}

static gdImage *_gd_img_scale_two_pass(gdImage *src,
                                    const unsigned int new_width,
                                    const unsigned int new_height)
{
    const unsigned int src_width = src->sx;
    const unsigned int src_height = src->sy;
    gdImage *tmp_im = NULL;
    gdImage *dst = NULL;
    int scale_pass_res;
    const FilterInfo *filter = _get_filterinfo_for_id(src->interpolation_id);

    // First, handle the trivial case.
    if (src_width == new_width && src_height == new_height)
    {
        return gd_img_copy(src);
    } // if

    // Convert to truecolor if it isn't; this code requires it.
    // if (!src->trueColor) {
    //     gdImagePaletteToTrueColor(src);
    // }// if

    // Scale horizontally unless sizes are the same.
    if (src_width == new_width)
    {
        tmp_im = src;
    }
    else
    {

        tmp_im = gd_img_new(new_width, src_height);
        if (tmp_im == NULL)
        {
            return NULL;
        }
        gd_img_set_interpolation_method(tmp_im, src->interpolation_id);

        scale_pass_res = _gdScalePass(src, src_width, tmp_im, new_width, src_height, HORIZONTAL, filter);
        if (scale_pass_res != 1)
        {
            gd_img_free(tmp_im);
            return NULL;
        }
    } // if .. else

    // If vertical sizes match, we're done.
    if (src_height == new_height)
    {
        assert(tmp_im != src);
        return tmp_im;
    } // if

    // Otherwise, we need to scale vertically.
    dst = gd_img_new(new_width, new_height);
    if (dst != NULL)
    {
        gd_img_set_interpolation_method(dst, src->interpolation_id);
        scale_pass_res = _gdScalePass(tmp_im, src_height, dst, new_height, new_width, VERTICAL, filter);
        if (scale_pass_res != 1)
        {
            gd_img_free(dst);
            if (src != tmp_im)
            {
                gd_img_free(tmp_im);
            }
            return NULL;
        }
    } // if

    if (src != tmp_im)
    {
        gd_img_free(tmp_im);
    } // if

    return dst;
} // gdImageScaleTwoPass

/*
        BilinearFixed, BicubicFixed and nearest implementations are
        rewamped versions of the implementation in CBitmapEx

        http://www.codeproject.com/Articles/29121/CBitmapEx-Free-C-Bitmap-Manipulation-Class

        Integer only implementation, good to have for common usages like
        pre scale very large images before using another interpolation
        methods for the last step.
*/
static gdImage *_gd_img_scale_nearest_neighbour(gdImage *im,
                                                const unsigned int width,
                                                const unsigned int height)
{
    const unsigned long new_width = MAX(1, width);
    const unsigned long new_height = MAX(1, height);
    const float dx = (float)im->sx / (float)new_width;
    const float dy = (float)im->sy / (float)new_height;
    const gdFixed f_dx = gd_ftofx(dx);
    const gdFixed f_dy = gd_ftofx(dy);

    gdImage *dst_img;
    unsigned long dst_offset_x;
    unsigned long dst_offset_y = 0;
    unsigned int i;

    dst_img = gd_img_new(new_width, new_height);

    if (dst_img == NULL)
    {
        return NULL;
    }

    for (i = 0; i < new_height; i++)
    {
        unsigned int j;
        dst_offset_x = 0;

        for (j = 0; j < new_width; j++)
        {
            const gdFixed f_i = gd_itofx(i);
            const gdFixed f_j = gd_itofx(j);
            const gdFixed f_a = gd_mulfx(f_i, f_dy);
            const gdFixed f_b = gd_mulfx(f_j, f_dx);
            const long m = gd_fxtoi(f_a);
            const long n = gd_fxtoi(f_b);

            dst_img->tpixels[dst_offset_y][dst_offset_x++] = im->tpixels[m][n];
        }

        dst_offset_y++;
    }

    return dst_img;
}

static gdImage *gdImageScaleBilinearTC(gdImage *im, const unsigned int new_width, const unsigned int new_height)
{
    long dst_w = MAX(1, new_width);
    long dst_h = MAX(1, new_height);
    float dx = (float)gd_img_sx(im) / (float)dst_w;
    float dy = (float)gd_img_sy(im) / (float)dst_h;
    gdFixed f_dx = gd_ftofx(dx);
    gdFixed f_dy = gd_ftofx(dy);
    gdFixed f_1 = gd_itofx(1);

    int dst_offset_h;
    int dst_offset_v = 0;
    long i;
    gdImage *new_img;

    new_img = gd_img_new(new_width, new_height);
    if (!new_img)
    {
        return NULL;
    }

    for (i = 0; i < dst_h; i++)
    {
        long j;
        dst_offset_h = 0;
        for (j = 0; j < dst_w; j++)
        {
            // Update bitmap
            gdFixed f_i = gd_itofx(i);
            gdFixed f_j = gd_itofx(j);
            gdFixed f_a = gd_mulfx(f_i, f_dy);
            gdFixed f_b = gd_mulfx(f_j, f_dx);
            const long m = gd_fxtoi(f_a);
            const long n = gd_fxtoi(f_b);
            gdFixed f_f = f_a - gd_itofx(m);
            gdFixed f_g = f_b - gd_itofx(n);

            const gdFixed f_w1 = gd_mulfx(f_1 - f_f, f_1 - f_g);
            const gdFixed f_w2 = gd_mulfx(f_1 - f_f, f_g);
            const gdFixed f_w3 = gd_mulfx(f_f, f_1 - f_g);
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
            pixel1 = getPixelOverflowTC(im, n, m, 0);
            pixel2 = getPixelOverflowTC(im, n + 1, m, pixel1);
            pixel3 = getPixelOverflowTC(im, n, m + 1, pixel1);
            pixel4 = getPixelOverflowTC(im, n + 1, m + 1, pixel1);

            f_r1 = gd_itofx(gd_get_red(pixel1));
            f_r2 = gd_itofx(gd_get_red(pixel2));
            f_r3 = gd_itofx(gd_get_red(pixel3));
            f_r4 = gd_itofx(gd_get_red(pixel4));
            f_g1 = gd_itofx(gd_get_green(pixel1));
            f_g2 = gd_itofx(gd_get_green(pixel2));
            f_g3 = gd_itofx(gd_get_green(pixel3));
            f_g4 = gd_itofx(gd_get_green(pixel4));
            f_b1 = gd_itofx(gd_get_blue(pixel1));
            f_b2 = gd_itofx(gd_get_blue(pixel2));
            f_b3 = gd_itofx(gd_get_blue(pixel3));
            f_b4 = gd_itofx(gd_get_blue(pixel4));
            f_a1 = gd_itofx(gd_get_alpha(pixel1));
            f_a2 = gd_itofx(gd_get_alpha(pixel2));
            f_a3 = gd_itofx(gd_get_alpha(pixel3));
            f_a4 = gd_itofx(gd_get_alpha(pixel4));
            {
                const unsigned char red = (unsigned char)gd_fxtoi(gd_mulfx(f_w1, f_r1) + gd_mulfx(f_w2, f_r2) + gd_mulfx(f_w3, f_r3) + gd_mulfx(f_w4, f_r4));
                const unsigned char green = (unsigned char)gd_fxtoi(gd_mulfx(f_w1, f_g1) + gd_mulfx(f_w2, f_g2) + gd_mulfx(f_w3, f_g3) + gd_mulfx(f_w4, f_g4));
                const unsigned char blue = (unsigned char)gd_fxtoi(gd_mulfx(f_w1, f_b1) + gd_mulfx(f_w2, f_b2) + gd_mulfx(f_w3, f_b3) + gd_mulfx(f_w4, f_b4));
                const unsigned char alpha = (unsigned char)gd_fxtoi(gd_mulfx(f_w1, f_a1) + gd_mulfx(f_w2, f_a2) + gd_mulfx(f_w3, f_a3) + gd_mulfx(f_w4, f_a4));

                new_img->tpixels[dst_offset_v][dst_offset_h] = gd_set_alpha(red, green, blue, alpha);
            }

            dst_offset_h++;
        }

        dst_offset_v++;
    }
    return new_img;
}

static gdImage *_gd_img_scale_bilinear(gdImage *im, const unsigned int new_width,
                                       const unsigned int new_height)
{
    return gdImageScaleBilinearTC(im, new_width, new_height);
}

static gdImage *_gd_img_scale_bicubic_fixed(gdImage *src, const unsigned int width,
                                            const unsigned int height)
{
    const long new_width = MAX(1, width);
    const long new_height = MAX(1, height);
    const int src_w = gd_img_sx(src);
    const int src_h = gd_img_sy(src);
    const gdFixed f_dx = gd_ftofx((float)src_w / (float)new_width);
    const gdFixed f_dy = gd_ftofx((float)src_h / (float)new_height);
    const gdFixed f_1 = gd_itofx(1);
    const gdFixed f_2 = gd_itofx(2);
    const gdFixed f_4 = gd_itofx(4);
    const gdFixed f_6 = gd_itofx(6);
    const gdFixed f_gamma = gd_ftofx(1.04f);
    gdImage *dst;

    unsigned int dst_offset_x;
    unsigned int dst_offset_y = 0;
    long i;

    dst = gd_img_new(new_width, new_height);
    if (!dst)
    {
        return NULL;
    }

    for (i = 0; i < new_height; i++)
    {
        long j;
        dst_offset_x = 0;

        for (j = 0; j < new_width; j++)
        {
            const gdFixed f_a = gd_mulfx(gd_itofx(i), f_dy);
            const gdFixed f_b = gd_mulfx(gd_itofx(j), f_dx);
            const long m = gd_fxtoi(f_a);
            const long n = gd_fxtoi(f_b);
            const gdFixed f_f = f_a - gd_itofx(m);
            const gdFixed f_g = f_b - gd_itofx(n);
            unsigned int src_offset_x[16], src_offset_y[16];
            long k;
            register gdFixed f_red = 0, f_green = 0, f_blue = 0, f_alpha = 0;
            unsigned char red, green, blue, alpha = 0;
            uint32_t *dst_row = dst->tpixels[dst_offset_y];

            if ((m < 1) || (n < 1))
            {
                src_offset_x[0] = n;
                src_offset_y[0] = m;
            }
            else
            {
                src_offset_x[0] = n - 1;
                src_offset_y[0] = m;
            }

            src_offset_x[1] = n;
            src_offset_y[1] = m;

            if ((m < 1) || (n >= src_w - 1))
            {
                src_offset_x[2] = n;
                src_offset_y[2] = m;
            }
            else
            {
                src_offset_x[2] = n + 1;
                src_offset_y[2] = m;
            }

            if ((m < 1) || (n >= src_w - 2))
            {
                src_offset_x[3] = n;
                src_offset_y[3] = m;
            }
            else
            {
                src_offset_x[3] = n + 1 + 1;
                src_offset_y[3] = m;
            }

            if (n < 1)
            {
                src_offset_x[4] = n;
                src_offset_y[4] = m;
            }
            else
            {
                src_offset_x[4] = n - 1;
                src_offset_y[4] = m;
            }

            src_offset_x[5] = n;
            src_offset_y[5] = m;
            if (n >= src_w - 1)
            {
                src_offset_x[6] = n;
                src_offset_y[6] = m;
            }
            else
            {
                src_offset_x[6] = n + 1;
                src_offset_y[6] = m;
            }

            if (n >= src_w - 2)
            {
                src_offset_x[7] = n;
                src_offset_y[7] = m;
            }
            else
            {
                src_offset_x[7] = n + 1 + 1;
                src_offset_y[7] = m;
            }

            if ((m >= src_h - 1) || (n < 1))
            {
                src_offset_x[8] = n;
                src_offset_y[8] = m;
            }
            else
            {
                src_offset_x[8] = n - 1;
                src_offset_y[8] = m;
            }

            src_offset_x[9] = n;
            src_offset_y[9] = m;

            if ((m >= src_h - 1) || (n >= src_w - 1))
            {
                src_offset_x[10] = n;
                src_offset_y[10] = m;
            }
            else
            {
                src_offset_x[10] = n + 1;
                src_offset_y[10] = m;
            }

            if ((m >= src_h - 1) || (n >= src_w - 2))
            {
                src_offset_x[11] = n;
                src_offset_y[11] = m;
            }
            else
            {
                src_offset_x[11] = n + 1 + 1;
                src_offset_y[11] = m;
            }

            if ((m >= src_h - 2) || (n < 1))
            {
                src_offset_x[12] = n;
                src_offset_y[12] = m;
            }
            else
            {
                src_offset_x[12] = n - 1;
                src_offset_y[12] = m;
            }

            if (!(m >= src_h - 2))
            {
                src_offset_x[13] = n;
                src_offset_y[13] = m;
            }

            if ((m >= src_h - 2) || (n >= src_w - 1))
            {
                src_offset_x[14] = n;
                src_offset_y[14] = m;
            }
            else
            {
                src_offset_x[14] = n + 1;
                src_offset_y[14] = m;
            }

            if ((m >= src_h - 2) || (n >= src_w - 2))
            {
                src_offset_x[15] = n;
                src_offset_y[15] = m;
            }
            else
            {
                src_offset_x[15] = n + 1 + 1;
                src_offset_y[15] = m;
            }

            for (k = -1; k < 3; k++)
            {
                const gdFixed f = gd_itofx(k) - f_f;
                const gdFixed f_fm1 = f - f_1;
                const gdFixed f_fp1 = f + f_1;
                const gdFixed f_fp2 = f + f_2;
                register gdFixed f_a = 0, f_b = 0, f_d = 0, f_c = 0;
                register gdFixed f_RY;
                int l;

                if (f_fp2 > 0)
                    f_a = gd_mulfx(f_fp2, gd_mulfx(f_fp2, f_fp2));
                if (f_fp1 > 0)
                    f_b = gd_mulfx(f_fp1, gd_mulfx(f_fp1, f_fp1));
                if (f > 0)
                    f_c = gd_mulfx(f, gd_mulfx(f, f));
                if (f_fm1 > 0)
                    f_d = gd_mulfx(f_fm1, gd_mulfx(f_fm1, f_fm1));

                f_RY = gd_divfx((f_a - gd_mulfx(f_4, f_b) + gd_mulfx(f_6, f_c) - gd_mulfx(f_4, f_d)), f_6);

                for (l = -1; l < 3; l++)
                {
                    const gdFixed f = gd_itofx(l) - f_g;
                    const gdFixed f_fm1 = f - f_1;
                    const gdFixed f_fp1 = f + f_1;
                    const gdFixed f_fp2 = f + f_2;
                    register gdFixed f_a = 0, f_b = 0, f_c = 0, f_d = 0;
                    register gdFixed f_RX, f_R, f_rs, f_gs, f_bs, f_ba;
                    register uint32_t c;
                    const int _k = ((k + 1) * 4) + (l + 1);

                    if (f_fp2 > 0)
                        f_a = gd_mulfx(f_fp2, gd_mulfx(f_fp2, f_fp2));

                    if (f_fp1 > 0)
                        f_b = gd_mulfx(f_fp1, gd_mulfx(f_fp1, f_fp1));

                    if (f > 0)
                        f_c = gd_mulfx(f, gd_mulfx(f, f));

                    if (f_fm1 > 0)
                        f_d = gd_mulfx(f_fm1, gd_mulfx(f_fm1, f_fm1));

                    f_RX = gd_divfx((f_a - gd_mulfx(f_4, f_b) + gd_mulfx(f_6, f_c) - gd_mulfx(f_4, f_d)), f_6);
                    f_R = gd_mulfx(f_RY, f_RX);

                    c = src->tpixels[*(src_offset_y + _k)][*(src_offset_x + _k)];
                    f_rs = gd_itofx(gd_get_red(c));
                    f_gs = gd_itofx(gd_get_green(c));
                    f_bs = gd_itofx(gd_get_blue(c));
                    f_ba = gd_itofx(gd_get_alpha(c));

                    f_red += gd_mulfx(f_rs, f_R);
                    f_green += gd_mulfx(f_gs, f_R);
                    f_blue += gd_mulfx(f_bs, f_R);
                    f_alpha += gd_mulfx(f_ba, f_R);
                }
            }

            red = (unsigned char)CLAMP(gd_fxtoi(gd_mulfx(f_red, f_gamma)), 0, 255);
            green = (unsigned char)CLAMP(gd_fxtoi(gd_mulfx(f_green, f_gamma)), 0, 255);
            blue = (unsigned char)CLAMP(gd_fxtoi(gd_mulfx(f_blue, f_gamma)), 0, 255);
            alpha = (unsigned char)CLAMP(gd_fxtoi(gd_mulfx(f_alpha, f_gamma)), 0, 255);

            *(dst_row + dst_offset_x) = gd_set_alpha(red, green, blue, alpha);

            dst_offset_x++;
        }
        dst_offset_y++;
    }
    return dst;
}

/**
 * Group: Interpolation Method
 */

/**
 * Function: gdImageSetInterpolationMethod
 *
 * Set the interpolation method for subsequent operations
 *
 * Parameters:
 *   im - The image.
 *   id - The interpolation method.
 *
 * Returns:
 *   Non-zero on success, zero on failure.
 *
 * See also:
 *   - <gdInterpolationMethod>
 *   - <gdImageGetInterpolationMethod>
 */
int gd_img_set_interpolation_method(gdImage *im, gdInterpolationMethod id)
{
    if (im == NULL || (uintmax_t)id > GD_METHOD_COUNT)
    {
        return 0;
    }

    switch (id)
    {
    case GD_NEAREST_NEIGHBOUR:
    case GD_WEIGHTED4:
        im->interpolation = NULL;
        break;

    // generic versions
    // GD_BILINEAR_FIXED and GD_BICUBIC_FIXED are kept for BC reasons
    case GD_BILINEAR_FIXED:
    case GD_LINEAR:
        im->interpolation = _filter_linear;
        break;
    case GD_BELL:
        im->interpolation = filter_bell;
        break;
    case GD_BESSEL:
        im->interpolation = _filter_bessel;
        break;
    case GD_BICUBIC_FIXED:
    case GD_BICUBIC:
        // no interpolation as gdImageScale calls a dedicated function
        im->interpolation = NULL;
        break;
    case GD_BLACKMAN:
        im->interpolation = _filter_blackman;
        break;
    case GD_BOX:
        im->interpolation = _filter_box;
        break;
    case GD_BSPLINE:
        im->interpolation = filter_bspline;
        break;
    case GD_CATMULLROM:
        im->interpolation = _filter_catmullrom;
        break;
    case GD_GAUSSIAN:
        im->interpolation = filter_gaussian;
        break;
    case GD_GENERALIZED_CUBIC:
        im->interpolation = _filter_generalized_cubic;
        break;
    case GD_HERMITE:
        im->interpolation = filter_hermite;
        break;
    case GD_HAMMING:
        im->interpolation = filter_hamming;
        break;
    case GD_HANNING:
        im->interpolation = filter_hanning;
        break;
    case GD_MITCHELL:
        im->interpolation = filter_mitchell;
        break;
    case GD_POWER:
        im->interpolation = filter_power;
        break;
    case GD_QUADRATIC:
        im->interpolation = filter_quadratic;
        break;
    case GD_SINC:
        im->interpolation = _filter_sinc;
        break;
    case GD_TRIANGLE:
        im->interpolation = filter_triangle;
        break;
    case GD_LANCZOS3:
        im->interpolation = filter_lanczos3;
        break;
    case GD_LANCZOS8:
        im->interpolation = _filter_lanczos8;
        break;
    case GD_BLACKMAN_BESSEL:
        im->interpolation = _filter_blackman_bessel;
        break;
    case GD_BLACKMAN_SINC:
        im->interpolation = _filter_blackman_sinc;
        break;
    case GD_QUADRATIC_BSPLINE:
        im->interpolation = filter_quadratic_bspline;
        break;
    case GD_CUBIC_SPLINE:
        im->interpolation = _filter_cubic_spline;
        break;
    case GD_COSINE:
        im->interpolation = filter_cosine;
        break;
    case GD_WELSH:
        im->interpolation = filter_welsh;
        break;
    case GD_DEFAULT:
        id = GD_LINEAR;
        im->interpolation = _filter_linear;
        break;
    default:
        return 0;
    }
    im->interpolation_id = id;
    return 1;
}

/**
 * Function: gdImageGetInterpolationMethod
 *
 * Get the current interpolation method
 *
 * This is here so that the value can be read via a language or VM with an FFI
 * but no (portable) way to extract the value from the struct.
 *
 * Parameters:
 *   im - The image.
 *
 * Returns:
 *   The current interpolation method.
 *
 * See also:
 *   - <gdInterpolationMethod>
 *   - <gdImageSetInterpolationMethod>
 */
gdInterpolationMethod gd_img_get_interpolation_method(gdImage *im)
{
    return im->interpolation_id;
}
