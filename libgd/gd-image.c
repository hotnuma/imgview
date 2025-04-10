#include "gd-image.h"

#include "gd-helpers.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

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
    gdImage *img = gd_img_new(sx, sy);

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
    im->transparent = (-1);

    /* 2.0.2: alpha blending is now on by default, and saving of alpha is
       off by default. This allows font antialiasing to work as expected
       on the first try in JPEGs -- quite important -- and also allows
       for smaller PNGs when saving of alpha channel is not really
       desired, which it usually isn't! */
    //im->thick = 1;
    //im->AA = 0;
    im->cx1 = 0;
    im->cy1 = 0;
    im->cx2 = im->sx - 1;
    im->cy2 = im->sy - 1;
    //im->res_x = GD_RESOLUTION;
    //im->res_y = GD_RESOLUTION;

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


