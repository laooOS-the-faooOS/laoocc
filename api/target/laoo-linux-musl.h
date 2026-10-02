#ifndef LAOOCC_TARGET_LAOO_LINUX_MUSL_H
#define LAOOCC_TARGET_LAOO_LINUX_MUSL_H

#include "target/target.h"

#define LO_LAOO_LINUX_MUSL_TRIPLE "x86_64-laoo-linux-musl"

void lo_target_laoo_linux_musl_init(lo_target *target);

const char *lo_target_laoo_linux_musl_triple(void);

#endif /* LAOOCC_TARGET_LAOO_LINUX_MUSL_H */
