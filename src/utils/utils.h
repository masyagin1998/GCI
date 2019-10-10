#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#define STR_BUF_SIZE 1024

#define PREFIX_UNUSED(variable) ((void)variable)

#define SAFE_MALLOC(ptr, n) \
    do {                                                                \
        if (((ptr) = malloc(sizeof(*(ptr)) * (n))) == NULL) {           \
            fprintf(stderr, "[%s:%d] unable to malloc %lu bytes\n",     \
                    __FILE__, __LINE__, (size_t) (sizeof(*(ptr)) * (n))); \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while(0)

#define SAFE_CALLOC(ptr, n)                                             \
    do {                                                                \
        if (((ptr) = calloc((n), sizeof(*(ptr)))) == NULL) {            \
            fprintf(stderr, "[%s:%d] unable to calloc %lu bytes\n",     \
                    __FILE__, __LINE__, (size_t) (sizeof(*(ptr)) * (n))); \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while(0)

#define SAFE_REALLOC(ptr, n)                                            \
    do {                                                                \
        if (((ptr) = realloc((ptr), sizeof(*(ptr)) * (n))) == NULL) {   \
            fprintf(stderr, "[%s:%d] unable to realloc to %lu bytes\n", \
                    __FILE__, __LINE__, (size_t) (sizeof(*(ptr)) * (n))); \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while(0)

#define SAFE_FREE(ptr) \
    do {               \
        free(ptr);     \
        ptr = NULL;    \
    } while(0)

FILE*file_open(const char*fname, const char*mode);

/*
  This macro is not safe for expressions!!!
  Use it only for variables!!!
*/
#define PUSH_BACK(arr, len, cap, val)           \
    do {                                        \
        if ((len) == (cap)) {                   \
            (cap) *= 2;                         \
            SAFE_REALLOC(arr, cap);             \
        }                                       \
        (arr)[(len)] = (val);                   \
        (len)++;                                \
    } while(0)

#endif  /* UTILS_H_INCLUDED */
