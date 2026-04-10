#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <string.h>
#include <math.h>

#define FLT_EPSILON 1e-6f

#define mu_assert(message, test) do { \
    if (!(test)) { \
        static char buf[512]; \
        snprintf(buf, sizeof(buf), "%s\nAssertion: %s\nLocation: %s:%d", \
                 (message), #test, __FILE__, __LINE__); \
        return buf; \
    } \
} while (0)
#define mu_assert_equals(message, expected, actual) do { \
    if ((expected) != (actual)) { \
        static char buf[256]; \
        snprintf(buf, sizeof(buf), "%s\nExpected: %d\nActual: %d", message, (expected), (actual)); \
        return buf; \
    } \
} while (0)
#define mu_assert_not_equals(message, expected, actual) do { \
    if ((expected) == (actual)) { \
        static char buf[256]; \
        snprintf(buf, sizeof(buf), "%s\nExpected: %d != %d", message, (expected), (actual)); \
        return buf; \
    } \
} while (0)
#define mu_assert_null(message, actual) do { \
    if ((actual) != NULL) { \
        static char buf[256]; \
        snprintf(buf, sizeof(buf), "%s\nExpected == NULL\nActual: %p", message, (actual)); \
        return buf; \
    } \
} while (0)
#define mu_assert_not_null(message, actual) do { \
    if ((actual) == NULL) { \
        static char buf[256]; \
        snprintf(buf, sizeof(buf), "%s\nExpected != NULL\nActual: %p", message, (actual)); \
        return buf; \
    } \
} while (0)
#define mu_assert_equals_float(message, expected, actual) do { \
    if (fabs((expected) - (actual)) > FLT_EPSILON) { \
        static char buf[256]; \
        snprintf(buf, sizeof(buf), "%s\nExpected: %f\nActual: %f", message, (expected), (actual)); \
        return buf; \
    } \
} while (0)
#define mu_assert_string_equals(message, expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        static char buf[512]; \
        snprintf(buf, sizeof(buf), "%s\nExpected: \"%s\"\nActual: \"%s\"", message, (expected), (actual)); \
        return buf; \
    } \
} while (0)
#define mu_run_test(test) do { \
    printf("Running sub-test: %s\n", #test); \
    fflush(stdout); \
    char *message = test(); \
    tests_run++; \
    if (message) { \
        static char buf[1024]; \
        snprintf(buf, sizeof(buf), "Sub-test failed: %s\n%s", #test, message); \
        return buf; \
    } \
} while (0)
extern int tests_run;

#endif
