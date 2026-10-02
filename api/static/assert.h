#ifndef LAOOCC_STATIC_ASSERT_H
#define LAOOCC_STATIC_ASSERT_H

#define LO_STATIC_ASSERT(condition, message) \
    _Static_assert((condition), message)

#define LO_STATIC_ARRAY_COUNT(array) \
    (sizeof(array) / sizeof((array)[0]))

#endif /* LAOOCC_STATIC_ASSERT_H */
