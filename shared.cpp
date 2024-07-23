#include <stdlib.h>
#include <stdio.h>

#include "shared.h"

void error_exit(const char *error_message)
{
    fprintf(stderr, error_message);
    exit(1);    
}
