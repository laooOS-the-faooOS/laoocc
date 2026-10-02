#ifndef LAOOCC_TEXT_VIEW_H
#define LAOOCC_TEXT_VIEW_H

#include <stddef.h>

typedef struct lo_text_view {
    const char *data;
    size_t length;
} lo_text_view;

#define LO_TEXT_VIEW_EMPTY \
    ((lo_text_view){ NULL, 0 })

#define LO_TEXT_VIEW_LITERAL(string) \
    ((lo_text_view){ (string), sizeof(string) - 1 })

lo_text_view lo_text_view_make(const char *data, size_t length);

int lo_text_view_empty(lo_text_view view);

int lo_text_view_equal(lo_text_view lhs, lo_text_view rhs);

int lo_text_view_compare(lo_text_view lhs, lo_text_view rhs);

lo_text_view lo_text_view_subview(lo_text_view view,
                                  size_t position,
                                  size_t length);

#endif /* LAOOCC_TEXT_VIEW_H */
