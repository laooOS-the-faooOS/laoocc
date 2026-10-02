#include <stdio.h>

#include "core/core.h"

int main(void)
{
    lo_i32 signed_value = -42;
    lo_u64 unsigned_value = 42;
    lo_bool enabled = LO_TRUE;

    int values[] = { 1, 2, 3, 4 };

    printf("laoocc core test\n");
    printf("version: %s\n", LO_VERSION_STRING);
    printf("signed: %d\n", signed_value);
    printf("unsigned: %llu\n",
           (unsigned long long)unsigned_value);
    printf("boolean: %u\n", (unsigned)enabled);
    printf("array count: %zu\n", LO_ARRAY_COUNT(values));

    return LO_STATUS_OK;
}
