#include "config.h"
#include "gd-helpers.h"

#include <stdio.h>

int overflow2(int a, int b)
{
    if (a <= 0 || b <= 0)
    {
        gd_error_ex(GD_WARNING,
                    "one parameter to a memory allocation multiplication"
                    " is negative or zero, failing operation gracefully\n");
        return 1;
    }
    if (a > INT_MAX / b)
    {
        gd_error_ex(GD_WARNING,
                    "product of memory allocation multiplication would"
                    " exceed INT_MAX, failing operation gracefully\n");
        return 1;
    }
    return 0;
}

typedef void(*gdErrorMethod)(int, const char *, va_list);

static void gd_stderr_error(int priority, const char *format, va_list args)
{
    switch (priority)
    {
    case GD_ERROR:
        fputs("GD Error: ", stderr);
        break;
    case GD_WARNING:
        fputs("GD Warning: ", stderr);
        break;
    case GD_NOTICE:
        fputs("GD Notice: ", stderr);
        break;
    case GD_INFO:
        fputs("GD Info: ", stderr);
        break;
    case GD_DEBUG:
        fputs("GD Debug: ", stderr);
        break;
    }
    vfprintf(stderr, format, args);
    fflush(stderr);
}

static gdErrorMethod gd_error_method = gd_stderr_error;

static void _gd_error_ex(int priority, const char *format, va_list args)
{
    if (gd_error_method)
    {
        gd_error_method(priority, format, args);
    }
}

void gd_error_ex(int priority, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    _gd_error_ex(priority, format, args);
    va_end(args);
}


