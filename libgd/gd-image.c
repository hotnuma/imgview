#include "gd-image.h"

#include "gd-helpers.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

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
    gdImage *img = gd_img_new(sx, sy);

    for (int y = 0; y < sy; ++y)
    {
        for (int x = 0; x < sx; ++x)
        {
            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            img->tpixels[y][x] = gd_set_alpha(p[0], p[1], p[2], 255 /*p[3]*/);
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
            p[3] = gd_get_alpha(c);
        }
    }

    return pixbuf;
}

int gdImageBoundsSafe (gdImagePtr im, int x, int y)
{
    return gdImageBoundsSafeMacro (im, x, y);
}

gdImagePtr gd_img_new(int sx, int sy)
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

    im = (gdImage *) malloc (sizeof (gdImage));
    if (!im) {
        return 0;
    }

    memset (im, 0, sizeof (gdImage));

    im->tpixels = (int **) malloc (sizeof (int *) * sy);

    if (!im->tpixels) {
        free(im);
        return 0;
    }

    for (i = 0; (i < sy); i++) {
        im->tpixels[i] = (int *) calloc (sx, sizeof (int));
        if (!im->tpixels[i]) {
            /* 2.0.34 */
            i--;
            while (i >= 0) {
                free(im->tpixels[i]);
                i--;
            }
            free(im->tpixels);
            free(im);
            return 0;
        }
    }

    im->sx = sx;
    im->sy = sy;
    im->interpolation = NULL;
    im->interpolation_id = GD_BILINEAR_FIXED;

    im->cx1 = 0;
    im->cy1 = 0;
    im->cx2 = im->sx - 1;
    im->cy2 = im->sy - 1;

    im->transparent = (-1);

    return im;
}

void gd_img_free (gdImagePtr im)
{
    int i;

    if (im->tpixels)
    {
        for (i = 0; (i < im->sy); i++)
        {
            free (im->tpixels[i]);
        }
        free (im->tpixels);
    }

    free (im);
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

    dst = gd_img_new(src->sx , src->sy);

    if (dst == NULL)
    {
        return NULL;
    }

    for (i = 0; i < src->sy; i++)
    {
        for (x = 0; x < src->sx; x++)
        {
            dst->tpixels[i][x] = src->tpixels[i][x];
        }
    }

    dst->cx1 = src->cx1;
    dst->cy1 = src->cy1;
    dst->cx2 = src->cx2;
    dst->cy2 = src->cy2;

    dst->interpolation_id = src->interpolation_id;
    dst->interpolation    = src->interpolation;

    return dst;
}


