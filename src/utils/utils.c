#include "utils.h"

#include <string.h>

FILE*file_open(const char*fname, const char*mode)
{
    if (strcmp(fname, "stdin") == 0) {
        return stdin;
    } else if (strcmp(fname, "stdout") == 0) {
        return stdout;
    } else if (strcmp(fname, "stderr") == 0) {
        return stderr;
    } else {
        FILE*f = fopen(fname, mode);
        if (f == NULL) {
            fprintf(stderr, "Unable to open file \"%s\" with mode \"%s\"",
                    fname, mode);
            exit(EXIT_FAILURE);
        }
        return f;
    }
}
