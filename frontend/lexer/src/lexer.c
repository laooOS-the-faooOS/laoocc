#include "lexer_api.h"

void lo_lexer_init(
    lo_lexer *lexer,
    const char *source,
    size_t length
)
{
    lexer->source = source;
    lexer->length = length;
    lexer->offset = 0;
    lexer->line = 1;
    lexer->column = 1;
}
