#ifndef LAOOCC_TARGET_TRIPLE_H
#define LAOOCC_TARGET_TRIPLE_H

#include "target/abi.h"
#include "target/arch.h"

typedef struct lo_target_triple {
    lo_target_arch arch;
    lo_target_abi abi;

    const char *os;
    const char *vendor;
} lo_target_triple;

void lo_target_triple_init(lo_target_triple *triple);

int lo_target_triple_parse(lo_target_triple *triple,
                           const char *triple_string);

#endif /* LAOOCC_TARGET_TRIPLE_H */
