#include "config.h"
#include "gd-helpers.h"

#include <stdio.h>

// TBB: gd_strtok_r is not portable; provide an implementation

#define SEP_TEST (separators[*((unsigned char *) s)])

char* gd_strtok_r(char *s, const char *sep, char **state)
{
	char separators[256];
	char *result = 0;
	memset (separators, 0, sizeof (separators));
	while (*sep) {
		separators[*((const unsigned char *) sep)] = 1;
		sep++;
	}
	if (!s) {
		// Pick up where we left off
		s = *state;
	}
	// 1. EOS
	if (!(*s)) {
		*state = s;
		return 0;
	}
	// 2. Leading separators, if any
	if (SEP_TEST) {
		do {
			s++;
		} while (SEP_TEST);
		// 2a. EOS after separators only
		if (!(*s)) {
			*state = s;
			return 0;
		}
	}
	// 3. A token
	result = s;
	do {
		// 3a. Token at end of string
		if (!(*s)) {
			*state = s;
			return result;
		}
		s++;
	} while (!SEP_TEST);
	// 4. Terminate token and skip trailing separators
	*s = '\0';
	do {
		s++;
	} while (SEP_TEST);
	// 5. Return token
	*state = s;
	return result;
}

void* gd_calloc (size_t nmemb, size_t size)
{
	return calloc (nmemb, size);
}

void* gd_malloc (size_t size)
{
	return malloc (size);
}

void* gd_realloc (void *ptr, size_t size)
{
	return realloc (ptr, size);
}

/*
  Function: gdFree

    Frees memory that has been allocated by libgd functions.

	Unless more specialized functions exists (for instance, <gdImageDestroy>),
	all memory that has been allocated by public libgd functions has to be
	freed by calling <gdFree>, and not by free(3), because libgd internally
	doesn't use alloc(3) and friends but rather its own allocation functions,
	which are, however, not publicly available.

  Parameters:

	ptr - Pointer to the memory space to free. If it is NULL, no operation is
		  performed.

  Returns:

	Nothing.
*/
void gd_free (void *ptr)
{
	free (ptr);
}

int overflow2(int a, int b)
{
    if(a <= 0 || b <= 0) {
        gd_error_ex(GD_WARNING, "one parameter to a memory allocation multiplication is negative or zero, failing operation gracefully\n");
        return 1;
    }
    if(a > INT_MAX / b) {
        gd_error_ex(GD_WARNING, "product of memory allocation multiplication would exceed INT_MAX, failing operation gracefully\n");
        return 1;
    }
    return 0;
}


/**
 * Group: Error Handling
 */

static void gd_stderr_error(int priority, const char *format, va_list args)
{
    switch (priority) {
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

typedef void(*gdErrorMethod)(int, const char *, va_list);

void gdSetErrorMethod(gdErrorMethod);
void gdClearErrorMethod(void);


static gdErrorMethod gd_error_method = gd_stderr_error;

static void _gd_error_ex(int priority, const char *format, va_list args)
{
    if (gd_error_method) {
        gd_error_method(priority, format, args);
    }
}

void gd_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    _gd_error_ex(GD_WARNING, format, args);
    va_end(args);
}
void gd_error_ex(int priority, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    _gd_error_ex(priority, format, args);
    va_end(args);
}

/*
    Function: gdSetErrorMethod
*/
void gdSetErrorMethod(gdErrorMethod error_method)
{
    gd_error_method = error_method;
}

/*
    Function: gdClearErrorMethod
*/
void gdClearErrorMethod()
{
    gd_error_method = gd_stderr_error;
}


