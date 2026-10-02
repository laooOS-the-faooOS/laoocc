#ifndef LAOOCC_LINKER_DETECT_H
#define LAOOCC_LINKER_DETECT_H

#include "linker/options.h"

const char *lo_linker_name(lo_linker_kind kind);

const char *lo_linker_program(lo_linker_kind kind);

lo_linker_kind lo_linker_detect(const char *program);

#endif /* LAOOCC_LINKER_DETECT_H */
