#ifndef GD_HELPERS_H
#define GD_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <syslog.h>

#define GD_ERROR LOG_ERR
#define GD_WARNING LOG_WARNING
#define GD_NOTICE LOG_NOTICE
#define GD_INFO LOG_INFO
#define GD_DEBUG LOG_DEBUG

void gd_error_ex(int priority, const char *format, ...);

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif
#define MIN3(a,b,c) ((a)<(b)?(MIN(a,c)):(MIN(b,c)))
#define MAX3(a,b,c) ((a)<(b)?(MAX(b,c)):(MAX(a,c)))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Convert a double to an unsigned char, rounding to the nearest
 * integer and clamping the result between 0 and max.  The absolute
 * value of clr must be less than the maximum value of an unsigned
 * short. */
static inline unsigned char uchar_clamp(double clr, unsigned char max)
{
    unsigned short result;

    //assert(fabs(clr) <= SHRT_MAX);

    /* Casting a negative float to an unsigned short is undefined.
     * However, casting a float to a signed truncates toward zero and
     * casting a negative signed value to an unsigned of the same size
     * results in a bit-identical value (assuming twos-complement
     * arithmetic).	 This is what we want: all legal negative values
     * for clr will be greater than 255. */

    /* Convert and clamp. */
    result = (unsigned short)(short)(clr + 0.5);
    if (result > max)
    {
        result = (clr < 0) ? 0 : max;
    }

    return result;
}

/* Returns nonzero if multiplying the two quantities will
   result in integer overflow. Also returns nonzero if
   either quantity is negative. By Phil Knirsch based on
   netpbm fixes by Alan Cox. */

int overflow2(int a, int b);

#ifdef __cplusplus
}
#endif

#endif // GD_HELPERS_H


