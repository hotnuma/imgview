#include "config.h"
#include "gd-image.h"

#include "gd-helpers.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>


// creation / destruction -----------------------------------------------------

gdImage* gd_img_new(int sx, int sy)
{
    if (overflow2(sx, sy))
        return NULL;

    if (overflow2(sizeof(int*), sy))
        return NULL;

    if (overflow2(sizeof(int), sx))
        return NULL;

    gdImage* img = (gdImage*) malloc(sizeof(gdImage));
    if (!img)
        return NULL;

    memset(img, 0, sizeof(gdImage));

    img->tpixels = (uint32_t **) malloc(sizeof(uint32_t*) * sy);

    if (!img->tpixels)
    {
        free(img);
        return NULL;
    }

    for (int i = 0; i < sy; ++i)
    {
        img->tpixels[i] = (uint32_t*) calloc(sx, sizeof(uint32_t));

        if (!img->tpixels[i])
        {
            i--;
            while (i >= 0)
            {
                free(img->tpixels[i]);
                i--;
            }
            free(img->tpixels);
            free(img);
            return NULL;
        }
    }

    assert(img->has_alpha == false);

    img->sx = sx;
    img->sy = sy;
    img->interpolation_id = GD_BILINEAR_FIXED;

    img->cx2 = img->sx - 1;
    img->cy2 = img->sy - 1;

    img->transparent = (-1);

    return img;
}

gdImage* gd_img_new_from_pixbuf(GdkPixbuf *pixbuf)
{
    if (!pixbuf)
        return NULL;

    int sx = gdk_pixbuf_get_width(pixbuf);
    int sy = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    gboolean has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    gdImage *img = gd_img_new(sx, sy);

    if (!has_alpha)
    {
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

    printf("gd_img_new_from_pixbuf: has_alpha\n");

    for (int y = 0; y < sy; ++y)
    {
        for (int x = 0; x < sx; ++x)
        {
            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            img->tpixels[y][x] = gd_set_alpha(p[0], p[1], p[2], p[3]);
        }
    }

    img->has_alpha = true;

    return img;
}

void gd_img_free(gdImage* img)
{
    if (img->tpixels)
    {
        for (int i = 0; (i < img->sy); i++)
        {
            free (img->tpixels[i]);
        }
        free (img->tpixels);
    }

    free (img);
}


// ----------------------------------------------------------------------------

GdkPixbuf* gd_to_pixbuf(gdImage *img)
{
    g_return_val_if_fail(img != NULL, NULL);

    gboolean has_alpha = img->has_alpha;

    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                       has_alpha,
                                       8,
                                       img->sx, img->sy);

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    if (!has_alpha)
    {
        for (int y = 0; y < img->sy; ++y)
        {
            for (int x = 0; x < img->sx; ++x)
            {
                uint32_t c = img->tpixels[y][x];

                guchar *p = pixels + (y * rowstride) + (x * n_channels);
                p[0] = gd_get_red(c);
                p[1] = gd_get_green(c);
                p[2] = gd_get_blue(c);
                p[3] = 255;
            }
        }

        return pixbuf;
    }

    for (int y = 0; y < img->sy; ++y)
    {
        for (int x = 0; x < img->sx; ++x)
        {
            uint32_t c = img->tpixels[y][x];

            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            p[0] = gd_get_red(c);
            p[1] = gd_get_green(c);
            p[2] = gd_get_blue(c);
            p[3] = gd_get_alpha(c);
        }
    }

    return pixbuf;
}

gdImage* gd_img_copy(const gdImage *src)
{
    gdImage *dst = gd_img_new(src->sx , src->sy);

    if (dst == NULL)
        return NULL;

    register int i;
    register int x;

    for (i = 0; i < src->sy; ++i)
    {
        for (x = 0; x < src->sx; ++x)
        {
            dst->tpixels[i][x] = src->tpixels[i][x];
        }
    }

    dst->has_alpha = src->has_alpha;

    dst->cx1 = src->cx1;
    dst->cy1 = src->cy1;
    dst->cx2 = src->cx2;
    dst->cy2 = src->cy2;

    dst->interpolation_id = src->interpolation_id;
    dst->interpolation    = src->interpolation;

    return dst;
}


