/* aubio configuration for xLights native macOS build */
#ifndef AUBIO_CONFIG_H
#define AUBIO_CONFIG_H

#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STRING_H 1
#define HAVE_ERRNO_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STDARG_H 1
#define HAVE_COMPLEX_H 1
#define HAVE_C99_VARARGS_MACROS 1

/* Use Apple Accelerate framework for FFT (vDSP) on macOS */
#ifdef __APPLE__
#define HAVE_ACCELERATE 1
#endif

/* Single precision (float, not double) */
/* #undef HAVE_AUBIO_DOUBLE */

#endif /* AUBIO_CONFIG_H */
