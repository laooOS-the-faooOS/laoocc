#ifndef LAOOCC_ERROR_MESSAGE_H
#define LAOOCC_ERROR_MESSAGE_H

#include <stddef.h>

typedef struct lo_error_message {
    char   *data;
    size_t  length;
    size_t  capacity;
} lo_error_message;

void lo_error_message_init(
    lo_error_message *message
);

void lo_error_message_free(
    lo_error_message *message
);

void lo_error_message_clear(
    lo_error_message *message
);

int lo_error_message_write(
    lo_error_message *message,
    const char *data,
    size_t length
);

int lo_error_message_puts(
    lo_error_message *message,
    const char *string
);

const char *lo_error_message_data(
    const lo_error_message *message
);

size_t lo_error_message_size(
    const lo_error_message *message
);

#endif /* LAOOCC_ERROR_MESSAGE_H */
