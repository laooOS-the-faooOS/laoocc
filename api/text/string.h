#ifndef LAOOCC_TEXT_STRING_H
#define LAOOCC_TEXT_STRING_H

#include <stddef.h>

typedef struct lo_text_string {
    char   *data;
    size_t  length;
    size_t  capacity;
} lo_text_string;

void lo_text_string_init(lo_text_string *string);
void lo_text_string_free(lo_text_string *string);

int lo_text_string_reserve(lo_text_string *string, size_t capacity);
int lo_text_string_resize(lo_text_string *string, size_t length);

int lo_text_string_append(lo_text_string *string,
                          const char *data,
                          size_t length);

int lo_text_string_push(lo_text_string *string, char character);

int lo_text_string_insert(lo_text_string *string,
                          size_t position,
                          const char *data,
                          size_t length);

int lo_text_string_remove(lo_text_string *string,
                          size_t position,
                          size_t length);

void lo_text_string_clear(lo_text_string *string);

#endif /* LAOOCC_TEXT_STRING_H */
