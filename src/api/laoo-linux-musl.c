#include "target/laoo-linux-musl.h"

void lo_target_laoo_linux_musl_init(lo_target *target)
{
    if (!target)
        return;

    lo_target_configure(target,
                        LO_LAOO_LINUX_MUSL_TRIPLE);
}

const char *lo_target_laoo_linux_musl_triple(void)
{
    return LO_LAOO_LINUX_MUSL_TRIPLE;
}
