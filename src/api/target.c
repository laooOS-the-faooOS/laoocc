#include "target/target.h"

#include <string.h>

/* Architecture */

const char *lo_target_arch_name(lo_target_arch arch)
{
    switch (arch) {
    case LO_TARGET_ARCH_X86_64:
        return "x86_64";
    case LO_TARGET_ARCH_X86:
        return "x86";
    case LO_TARGET_ARCH_AARCH64:
        return "aarch64";
    case LO_TARGET_ARCH_ARM:
        return "arm";
    case LO_TARGET_ARCH_RISCV64:
        return "riscv64";
    case LO_TARGET_ARCH_RISCV32:
        return "riscv32";
    default:
        return "unknown";
    }
}

lo_target_arch lo_target_arch_detect(const char *name)
{
    if (!name)
        return LO_TARGET_ARCH_UNKNOWN;

    if (strcmp(name, "x86_64") == 0 ||
        strcmp(name, "amd64") == 0)
        return LO_TARGET_ARCH_X86_64;

    if (strcmp(name, "x86") == 0 ||
        strcmp(name, "i386") == 0 ||
        strcmp(name, "i686") == 0)
        return LO_TARGET_ARCH_X86;

    if (strcmp(name, "aarch64") == 0 ||
        strcmp(name, "arm64") == 0)
        return LO_TARGET_ARCH_AARCH64;

    if (strcmp(name, "arm") == 0)
        return LO_TARGET_ARCH_ARM;

    if (strcmp(name, "riscv64") == 0)
        return LO_TARGET_ARCH_RISCV64;

    if (strcmp(name, "riscv32") == 0)
        return LO_TARGET_ARCH_RISCV32;

    return LO_TARGET_ARCH_UNKNOWN;
}

/* ABI */

const char *lo_target_abi_name(lo_target_abi abi)
{
    switch (abi) {
    case LO_TARGET_ABI_SYSV:
        return "sysv";
    case LO_TARGET_ABI_MUSL:
        return "musl";
    case LO_TARGET_ABI_GNU:
        return "gnu";
    case LO_TARGET_ABI_EABI:
        return "eabi";
    case LO_TARGET_ABI_EABIH:
        return "eabih";
    default:
        return "unknown";
    }
}

lo_target_abi lo_target_abi_detect(const char *name)
{
    if (!name)
        return LO_TARGET_ABI_UNKNOWN;

    if (strcmp(name, "sysv") == 0)
        return LO_TARGET_ABI_SYSV;

    if (strcmp(name, "musl") == 0)
        return LO_TARGET_ABI_MUSL;

    if (strcmp(name, "gnu") == 0 ||
        strcmp(name, "glibc") == 0)
        return LO_TARGET_ABI_GNU;

    if (strcmp(name, "eabi") == 0)
        return LO_TARGET_ABI_EABI;

    if (strcmp(name, "eabih") == 0)
        return LO_TARGET_ABI_EABIH;

    return LO_TARGET_ABI_UNKNOWN;
}

/* Triple */

void lo_target_triple_init(lo_target_triple *triple)
{
    if (!triple)
        return;

    triple->arch = LO_TARGET_ARCH_UNKNOWN;
    triple->abi = LO_TARGET_ABI_UNKNOWN;
    triple->os = NULL;
    triple->vendor = NULL;
}

int lo_target_triple_parse(lo_target_triple *triple,
                           const char *triple_string)
{
    const char *first;
    const char *second;
    const char *third;
    const char *fourth;

    if (!triple || !triple_string)
        return -1;

    first = strchr(triple_string, '-');

    if (!first)
        return -1;

    second = strchr(first + 1, '-');

    if (!second)
        return -1;

    third = strchr(second + 1, '-');

    /*
     * Supported forms:
     *
     *   arch-os-abi
     *   arch-vendor-os-abi
     */

    if (!third) {
        triple->arch = lo_target_arch_detect(triple_string);

        if (triple->arch == LO_TARGET_ARCH_UNKNOWN) {
            char arch[32];
            size_t length = (size_t)(first - triple_string);

            if (length >= sizeof(arch))
                return -1;

            memcpy(arch, triple_string, length);
            arch[length] = '\0';

            triple->arch = lo_target_arch_detect(arch);
        }

        if (triple->arch == LO_TARGET_ARCH_UNKNOWN)
            return -1;

        {
            char os[32];
            char abi[32];
            size_t os_length =
                (size_t)(second - first - 1);

            size_t abi_length =
                strlen(second + 1);

            if (os_length >= sizeof(os) ||
                abi_length >= sizeof(abi))
                return -1;

            memcpy(os, first + 1, os_length);
            os[os_length] = '\0';

            memcpy(abi, second + 1, abi_length + 1);

            triple->os = triple_string + (first - triple_string) + 1;
            triple->vendor = NULL;
            triple->abi = lo_target_abi_detect(abi);
        }

        return 0;
    }

    fourth = strchr(third + 1, '-');

    if (fourth)
        return -1;

    {
        char arch[32];
        char vendor[32];
        char os[32];
        char abi[32];

        size_t arch_length =
            (size_t)(first - triple_string);

        size_t vendor_length =
            (size_t)(second - first - 1);

        size_t os_length =
            (size_t)(third - second - 1);

        size_t abi_length =
            strlen(third + 1);

        if (arch_length >= sizeof(arch) ||
            vendor_length >= sizeof(vendor) ||
            os_length >= sizeof(os) ||
            abi_length >= sizeof(abi))
            return -1;

        memcpy(arch, triple_string, arch_length);
        arch[arch_length] = '\0';

        memcpy(vendor, first + 1, vendor_length);
        vendor[vendor_length] = '\0';

        memcpy(os, second + 1, os_length);
        os[os_length] = '\0';

        memcpy(abi, third + 1, abi_length + 1);

        triple->arch = lo_target_arch_detect(arch);

        if (triple->arch == LO_TARGET_ARCH_UNKNOWN)
            return -1;

        triple->vendor = triple_string + arch_length + 1;
        triple->os = triple_string +
                     arch_length + vendor_length + 2;

        triple->abi = lo_target_abi_detect(abi);
    }

    return 0;
}

/* Target */

void lo_target_init(lo_target *target)
{
    if (!target)
        return;

    lo_target_triple_init(&target->triple);

    target->pointer_bits = 0;
    target->long_bits = 0;

    target->char_bits = 0;
    target->short_bits = 0;
    target->int_bits = 0;
    target->long_long_bits = 0;
}

int lo_target_configure(lo_target *target,
                        const char *triple)
{
    if (!target || !triple)
        return -1;

    lo_target_init(target);

    if (lo_target_triple_parse(&target->triple, triple) != 0)
        return -1;

    switch (target->triple.arch) {
    case LO_TARGET_ARCH_X86_64:
    case LO_TARGET_ARCH_AARCH64:
    case LO_TARGET_ARCH_RISCV64:
        target->pointer_bits = 64;
        target->long_bits = 64;
        break;

    case LO_TARGET_ARCH_X86:
    case LO_TARGET_ARCH_ARM:
    case LO_TARGET_ARCH_RISCV32:
        target->pointer_bits = 32;
        target->long_bits = 32;
        break;

    default:
        return -1;
    }

    target->char_bits = 8;
    target->short_bits = 16;
    target->int_bits = 32;
    target->long_long_bits = 64;

    return 0;
}

const char *lo_target_name(const lo_target *target)
{
    if (!target)
        return "unknown";

    return lo_target_arch_name(target->triple.arch);
}
