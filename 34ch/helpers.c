#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helpers.h"

long num_convert(char* num) {
    long result = 0L;
    char* remainder = NULL;

    result = strtol(num, &remainder, BASE);
    if (remainder && *remainder != '\0') {
        fprintf(stderr, "Invalid value for a number, violating part is: %s.\n", remainder);
        exit(EXIT_FAILURE);
    }
    return result;
}

void print_error(int errcode) {
    fprintf(stderr, "Error encountered: %s.\n", strerror(errcode));
}
