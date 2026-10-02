#ifndef LAOOCC_TARGET_TARGET_H
#define LAOOCC_TARGET_TARGET_H

#include "target/arch.h"
#include "target/abi.h"
#include "target/triple.h"

typedef struct lo_target {
    lo_target_triple triple;

    unsigned pointer_bits;
    unsigned long_bits;

    unsigned char_bits;
    unsigned short_bits;
    unsigned int_bits;
    unsigned long_long_bits;
} lo_target;

void lo_target_init(lo_target *target);

int lo_target_configure(lo_target *target,
                        const char *triple);

const char *lo_target_name(const lo_target *target);

#endif /* LAOOCC_TARGET_TARGET_H */
