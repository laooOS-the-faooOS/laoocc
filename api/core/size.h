#ifndef LAOOCC_CORE_SIZE_H
#define LAOOCC_CORE_SIZE_H

#include <stddef.h>
#include <stdint.h>

typedef size_t    lo_usize;
typedef ptrdiff_t lo_isize;

#define LO_SIZE_MAX SIZE_MAX
#define LO_SIZE_MIN ((lo_isize)PTRDIFF_MIN)

#endif /* LAOOCC_CORE_SIZE_H */
