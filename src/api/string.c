#include "core/string.h"

#include <string.h>

size_t lo_string_length(lo_string string)
{
    return string.length;
}

int lo_string_empty(lo_string string)
{
    return string.length == 0;
}

int lo_string_compare(lo_string lhs, lo_string rhs)
{
    size_t length;

    length = lhs.length < rhs.length ? lhs.length : rhs.length;

    if (length != 0) {
        int result = memcmp(lhs.data, rhs.data, length);

        if (result != 0)
            return result;
    }

    if (lhs.length < rhs.length)
        return -1;

    if (lhs.length > rhs.length)
        return 1;

    return 0;
}

int lo_string_equal(lo_string lhs, lo_string rhs)
{
    return lo_string_compare(lhs, rhs) == 0;
}
