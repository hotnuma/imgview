#ifndef GD_HELPERS_H
#define GD_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

# if defined(__GNUC__) || defined(__clang__)
#  define BGD_EXPORT_DATA_PROT __attribute__ ((__visibility__ ("default")))
#  define BGD_EXPORT_DATA_IMPL __attribute__ ((__visibility__ ("hidden")))
# else
#  define BGD_EXPORT_DATA_PROT
#  define BGD_EXPORT_DATA_IMPL
# endif
# define BGD_STDCALL
# define BGD_MALLOC __attribute__ ((__malloc__))

#include <sys/types.h>
#include <stdlib.h>

//#define MIN(a,b) ((a)<(b)?(a):(b))
//#define MAX(a,b) ((a)<(b)?(b):(a))

#define MIN3(a,b,c) ((a)<(b)?(MIN(a,c)):(MIN(b,c)))
#define MAX3(a,b,c) ((a)<(b)?(MAX(b,c)):(MAX(a,c)))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Convert a double to an unsigned char, rounding to the nearest
 * integer and clamping the result between 0 and max.  The absolute
 * value of clr must be less than the maximum value of an unsigned
 * short. */
static inline unsigned char
uchar_clamp(double clr, unsigned char max) {
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
    if (result > max) {
        result = (clr < 0) ? 0 : max;
    }/* if */

    return result;
}/* uchar_clamp*/



/* TBB: strtok_r is not universal; provide an implementation of it. */

char *gd_strtok_r(char *s, const char *sep, char **state);

/* These functions wrap memory management. gdFree is
    in gd.h, where callers can utilize it to correctly
    free memory allocated by these functions with the
    right version of free(). */
void *gdCalloc(size_t nmemb, size_t size) BGD_MALLOC;
void *gdMalloc(size_t size) BGD_MALLOC;
void *gdRealloc (void *ptr, size_t size);
/* The extended version of gdReallocEx will free *ptr if the
 * realloc fails */
void *gdReallocEx (void *ptr, size_t size);
/* Guaranteed to correctly free memory returned by the gdImage*Ptr
   functions */
void gdFree (void *m);

/* Returns nonzero if multiplying the two quantities will
    result in integer overflow. Also returns nonzero if
    either quantity is negative. By Phil Knirsch based on
    netpbm fixes by Alan Cox. */

int overflow2(int a, int b);

#if 0
/* 2.0.16: portable mutex support for thread safety. */
#if defined(CPP_SHARP)
# define gdMutexDeclare(x)
# define gdMutexSetup(x)
# define gdMutexShutdown(x)
# define gdMutexLock(x)
# define gdMutexUnlock(x)
#elif defined(_WIN32)
	/* 2.0.18: must include windows.h to get CRITICAL_SECTION. */
# include <windows.h>
# define gdMutexDeclare(x) CRITICAL_SECTION x
# define gdMutexSetup(x) InitializeCriticalSection(&x)
# define gdMutexShutdown(x) DeleteCriticalSection(&x)
# define gdMutexLock(x) EnterCriticalSection(&x)
# define gdMutexUnlock(x) LeaveCriticalSection(&x)
#elif defined(HAVE_PTHREAD)
# include <pthread.h>
# define gdMutexDeclare(x) pthread_mutex_t x
# define gdMutexSetup(x) pthread_mutex_init(&x, 0)
# define gdMutexShutdown(x) pthread_mutex_destroy(&x)
# define gdMutexLock(x) pthread_mutex_lock(&x)
# define gdMutexUnlock(x) pthread_mutex_unlock(&x)
#else
# define gdMutexDeclare(x)
# define gdMutexSetup(x)
# define gdMutexShutdown(x)
# define gdMutexLock(x)
# define gdMutexUnlock(x)
#endif /* _WIN32 || HAVE_PTHREAD */

#define DPCM2DPI(dpcm) (unsigned int)((dpcm)*2.54 + 0.5)
#define DPM2DPI(dpm)   (unsigned int)((dpm)*0.0254 + 0.5)
#define DPI2DPCM(dpi)  (unsigned int)((dpi)/2.54 + 0.5)
#define DPI2DPM(dpi)   (unsigned int)((dpi)/0.0254 + 0.5)


#endif

#ifdef __cplusplus
}
#endif

#endif // GD_HELPERS_H


