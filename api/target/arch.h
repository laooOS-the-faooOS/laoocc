#ifndef LAOOCC_TARGET_ARCH_H
#define LAOOCC_TARGET_ARCH_H

typedef enum lo_target_arch {
    LO_TARGET_ARCH_UNKNOWN = 0,

    LO_TARGET_ARCH_X86_64,
    LO_TARGET_ARCH_X86,

    LO_TARGET_ARCH_AARCH64,
    LO_TARGET_ARCH_ARM,

    LO_TARGET_ARCH_RISCV64,
    LO_TARGET_ARCH_RISCV32
} lo_target_arch;

const char *lo_target_arch_name(lo_target_arch arch);

lo_target_arch lo_target_arch_detect(const char *name);

#endif /* LAOOCC_TARGET_ARCH_H */
