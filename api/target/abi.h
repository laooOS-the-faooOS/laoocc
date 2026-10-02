#ifndef LAOOCC_TARGET_ABI_H
#define LAOOCC_TARGET_ABI_H

typedef enum lo_target_abi {
    LO_TARGET_ABI_UNKNOWN = 0,

    LO_TARGET_ABI_SYSV,
    LO_TARGET_ABI_MUSL,
    LO_TARGET_ABI_GNU,

    LO_TARGET_ABI_EABI,
    LO_TARGET_ABI_EABIH
} lo_target_abi;

const char *lo_target_abi_name(lo_target_abi abi);

lo_target_abi lo_target_abi_detect(const char *name);

#endif /* LAOOCC_TARGET_ABI_H */
