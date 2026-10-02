/*
 * laoocc lexer: single-pass manual scanner. No allocation, no copying.
 *
 * Return convention of lo_lexer_next():
 *     0  a token was produced
 *     1  end of input (token->kind == LO_LEXER_TOKEN_EOF)
 *    -1  lexer error (error is filled in; token is left untouched)
 *
 * On error the cursor is not advanced past the offending input, so calling
 * lo_lexer_next() again reports the same error. Callers should stop.
 *
 * Tokens never contain newlines (strings and characters may not span
 * lines), so after a token the column advances by its length.
 */

#include "lexer.h"

#include <string.h>

#define LO_RESULT_TOKEN 0
#define LO_RESULT_EOF 1
#define LO_RESULT_ERROR (-1)

void lo_lexer_init(lo_lexer *lexer, const char *source, size_t length)
{
    lexer->source = source;
    lexer->length = length;
    lexer->offset = 0;
    lexer->line = 1;
    lexer->column = 1;
}

/* ---------------------------------------------------------------- helpers */

static int is_ident_start(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(unsigned char c)
{
    return c >= '0' && c <= '9';
}

static int is_ident_part(unsigned char c)
{
    return is_ident_start(c) || is_digit(c);
}

static int set_error(lo_lexer_error *error, lo_lexer_error_kind kind,
                     const char *message, size_t line, size_t column)
{
    error->kind = kind;
    error->message = message;
    error->line = (unsigned long)line;
    error->column = (unsigned long)column;
    return LO_RESULT_ERROR;
}

/* Fill the token and advance the cursor past it (single-line tokens only). */
static int emit(lo_lexer *lx, lo_token *tok, lo_token_kind kind, size_t len)
{
    tok->kind = kind;
    tok->start = lx->source + lx->offset;
    tok->length = len;
    tok->line = lx->line;
    tok->column = lx->column;
    lx->offset += len;
    lx->column += len;
    return LO_RESULT_TOKEN;
}

/* --------------------------------------------------------------- trivia */

/* Skips whitespace and comments. Returns 0, or -1 on unterminated comment. */
static int skip_trivia(lo_lexer *lx, lo_lexer_error *error)
{
    const char *s = lx->source;
    size_t n = lx->length;
    size_t i = lx->offset;
    size_t line = lx->line;
    size_t col = lx->column;

    while (i < n) {
        char c = s[i];

        if (c == ' ' || c == '\t' || c == '\r') {
            i++;
            col++;
        } else if (c == '\n') {
            i++;
            line++;
            col = 1;
        } else if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            i += 2;
            col += 2;
            while (i < n && s[i] != '\n') {
                i++;
                col++;
            }
        } else if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            size_t start_line = line;
            size_t start_col = col;

            i += 2;
            col += 2;
            for (;;) {
                if (i >= n) {
                    return set_error(error, LO_LEXER_ERROR_UNTERMINATED_COMMENT,
                                     "unterminated block comment",
                                     start_line, start_col);
                }
                if (s[i] == '*' && i + 1 < n && s[i + 1] == '/') {
                    i += 2;
                    col += 2;
                    break;
                }
                if (s[i] == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                i++;
            }
        } else {
            break;
        }
    }

    lx->offset = i;
    lx->line = line;
    lx->column = col;
    return 0;
}

/* ------------------------------------------------------------- keywords */

#define KW(text, kind) \
    if (len == sizeof(text) - 1 && memcmp(s, text, sizeof(text) - 1) == 0) \
        return kind

/* Dispatch on first character, then compare length + bytes. */
static lo_token_kind keyword_kind(const char *s, size_t len)
{
    if (len < 2 || len > 8)
        return LO_LEXER_TOKEN_IDENTIFIER;

    switch (s[0]) {
    case 'a': KW("auto", LO_LEXER_TOKEN_KW_AUTO); break;
    case 'b': KW("break", LO_LEXER_TOKEN_KW_BREAK); break;
    case 'c':
        KW("case", LO_LEXER_TOKEN_KW_CASE);
        KW("char", LO_LEXER_TOKEN_KW_CHAR);
        KW("const", LO_LEXER_TOKEN_KW_CONST);
        KW("continue", LO_LEXER_TOKEN_KW_CONTINUE);
        break;
    case 'd':
        KW("default", LO_LEXER_TOKEN_KW_DEFAULT);
        KW("do", LO_LEXER_TOKEN_KW_DO);
        KW("double", LO_LEXER_TOKEN_KW_DOUBLE);
        break;
    case 'e':
        KW("else", LO_LEXER_TOKEN_KW_ELSE);
        KW("enum", LO_LEXER_TOKEN_KW_ENUM);
        KW("extern", LO_LEXER_TOKEN_KW_EXTERN);
        break;
    case 'f':
        KW("float", LO_LEXER_TOKEN_KW_FLOAT);
        KW("for", LO_LEXER_TOKEN_KW_FOR);
        break;
    case 'g': KW("goto", LO_LEXER_TOKEN_KW_GOTO); break;
    case 'i':
        KW("if", LO_LEXER_TOKEN_KW_IF);
        KW("int", LO_LEXER_TOKEN_KW_INT);
        break;
    case 'l': KW("long", LO_LEXER_TOKEN_KW_LONG); break;
    case 'r':
        KW("register", LO_LEXER_TOKEN_KW_REGISTER);
        KW("return", LO_LEXER_TOKEN_KW_RETURN);
        break;
    case 's':
        KW("short", LO_LEXER_TOKEN_KW_SHORT);
        KW("signed", LO_LEXER_TOKEN_KW_SIGNED);
        KW("sizeof", LO_LEXER_TOKEN_KW_SIZEOF);
        KW("static", LO_LEXER_TOKEN_KW_STATIC);
        KW("struct", LO_LEXER_TOKEN_KW_STRUCT);
        KW("switch", LO_LEXER_TOKEN_KW_SWITCH);
        break;
    case 't': KW("typedef", LO_LEXER_TOKEN_KW_TYPEDEF); break;
    case 'u':
        KW("union", LO_LEXER_TOKEN_KW_UNION);
        KW("unsigned", LO_LEXER_TOKEN_KW_UNSIGNED);
        break;
    case 'v':
        KW("void", LO_LEXER_TOKEN_KW_VOID);
        KW("volatile", LO_LEXER_TOKEN_KW_VOLATILE);
        break;
    case 'w': KW("while", LO_LEXER_TOKEN_KW_WHILE); break;
    default: break;
    }
    return LO_LEXER_TOKEN_IDENTIFIER;
}

#undef KW

/* ------------------------------------------------------------- scanners */

static int scan_identifier(lo_lexer *lx, lo_token *tok)
{
    const char *s = lx->source + lx->offset;
    size_t max = lx->length - lx->offset;
    size_t len = 1;

    while (len < max && is_ident_part((unsigned char)s[len]))
        len++;

    return emit(lx, tok, keyword_kind(s, len), len);
}

/*
 * Numbers are delimited like a C "pp-number": digits, letters, '_', '.', and
 * a sign directly after an exponent marker (e/E, plus p/P for hex). That
 * covers decimal, 0x hex, octal, floats and suffixes (u, l, ul, f, ...)
 * without validating them; validation belongs to a later stage.
 */
static int scan_number(lo_lexer *lx, lo_token *tok)
{
    const char *s = lx->source + lx->offset;
    size_t max = lx->length - lx->offset;
    size_t len = 1;
    int hex = (s[0] == '0' && max > 1 && (s[1] == 'x' || s[1] == 'X'));

    while (len < max) {
        unsigned char c = (unsigned char)s[len];

        if (is_ident_part(c) || c == '.') {
            len++;
        } else if ((c == '+' || c == '-') &&
                   (hex ? (s[len - 1] == 'p' || s[len - 1] == 'P')
                        : (s[len - 1] == 'e' || s[len - 1] == 'E'))) {
            len++;
        } else {
            break;
        }
    }

    return emit(lx, tok, LO_LEXER_TOKEN_NUMBER, len);
}

/* Shared by strings and character literals; quote is '"' or '\''. */
static int scan_quoted(lo_lexer *lx, lo_token *tok, lo_lexer_error *error,
                       char quote)
{
    const char *s = lx->source + lx->offset;
    size_t max = lx->length - lx->offset;
    size_t len = 1;

    while (len < max) {
        char c = s[len];

        if (c == quote)
            return emit(lx, tok,
                        quote == '"' ? LO_LEXER_TOKEN_STRING
                                     : LO_LEXER_TOKEN_CHARACTER,
                        len + 1);
        if (c == '\n')
            break;
        if (c == '\\') {
            /* Skip the escaped character; an escaped newline/EOF is an error. */
            if (len + 1 >= max || s[len + 1] == '\n')
                break;
            len++;
        }
        len++;
    }

    if (quote == '"')
        return set_error(error, LO_LEXER_ERROR_UNTERMINATED_STRING,
                         "unterminated string literal", lx->line, lx->column);
    return set_error(error, LO_LEXER_ERROR_UNTERMINATED_CHARACTER,
                     "unterminated character literal", lx->line, lx->column);
}

static int scan_operator(lo_lexer *lx, lo_token *tok, lo_lexer_error *error)
{
    const char *s = lx->source + lx->offset;
    char c = s[0];
    char next = (lx->offset + 1 < lx->length) ? s[1] : '\0';

    switch (c) {
    case '+':
        return next == '+' ? emit(lx, tok, LO_LEXER_TOKEN_INCREMENT, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_PLUS, 1);
    case '-':
        if (next == '-') return emit(lx, tok, LO_LEXER_TOKEN_DECREMENT, 2);
        if (next == '>') return emit(lx, tok, LO_LEXER_TOKEN_ARROW, 2);
        return emit(lx, tok, LO_LEXER_TOKEN_MINUS, 1);
    case '&':
        return next == '&' ? emit(lx, tok, LO_LEXER_TOKEN_AND, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_AMPERSAND, 1);
    case '|':
        return next == '|' ? emit(lx, tok, LO_LEXER_TOKEN_OR, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_PIPE, 1);
    case '=':
        return next == '=' ? emit(lx, tok, LO_LEXER_TOKEN_EQUAL, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_ASSIGN, 1);
    case '!':
        return next == '=' ? emit(lx, tok, LO_LEXER_TOKEN_NOT_EQUAL, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_BANG, 1);
    case '<':
        return next == '=' ? emit(lx, tok, LO_LEXER_TOKEN_LESS_EQUAL, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_LESS, 1);
    case '>':
        return next == '=' ? emit(lx, tok, LO_LEXER_TOKEN_GREATER_EQUAL, 2)
                           : emit(lx, tok, LO_LEXER_TOKEN_GREATER, 1);

    case '*': return emit(lx, tok, LO_LEXER_TOKEN_STAR, 1);
    case '/': return emit(lx, tok, LO_LEXER_TOKEN_SLASH, 1);
    case '%': return emit(lx, tok, LO_LEXER_TOKEN_PERCENT, 1);
    case '^': return emit(lx, tok, LO_LEXER_TOKEN_CARET, 1);
    case '~': return emit(lx, tok, LO_LEXER_TOKEN_TILDE, 1);

    case '(': return emit(lx, tok, LO_LEXER_TOKEN_LPAREN, 1);
    case ')': return emit(lx, tok, LO_LEXER_TOKEN_RPAREN, 1);
    case '{': return emit(lx, tok, LO_LEXER_TOKEN_LBRACE, 1);
    case '}': return emit(lx, tok, LO_LEXER_TOKEN_RBRACE, 1);
    case '[': return emit(lx, tok, LO_LEXER_TOKEN_LBRACKET, 1);
    case ']': return emit(lx, tok, LO_LEXER_TOKEN_RBRACKET, 1);
    case ',': return emit(lx, tok, LO_LEXER_TOKEN_COMMA, 1);
    case '.': return emit(lx, tok, LO_LEXER_TOKEN_DOT, 1);
    case ';': return emit(lx, tok, LO_LEXER_TOKEN_SEMICOLON, 1);
    case ':': return emit(lx, tok, LO_LEXER_TOKEN_COLON, 1);
    case '?': return emit(lx, tok, LO_LEXER_TOKEN_QUESTION, 1);

    default:
        return set_error(error, LO_LEXER_ERROR_INVALID_CHARACTER,
                         "invalid character", lx->line, lx->column);
    }
}

/* ------------------------------------------------------------ public API */

int lo_lexer_next(lo_lexer *lexer, lo_token *token, lo_lexer_error *error)
{
    unsigned char c;

    if (skip_trivia(lexer, error) < 0)
        return LO_RESULT_ERROR;

    if (lexer->offset >= lexer->length) {
        token->kind = LO_LEXER_TOKEN_EOF;
        token->start = lexer->source + lexer->offset;
        token->length = 0;
        token->line = lexer->line;
        token->column = lexer->column;
        return LO_RESULT_EOF;
    }

    c = (unsigned char)lexer->source[lexer->offset];

    if (is_ident_start(c))
        return scan_identifier(lexer, token);

    if (is_digit(c) ||
        (c == '.' && lexer->offset + 1 < lexer->length &&
         is_digit((unsigned char)lexer->source[lexer->offset + 1])))
        return scan_number(lexer, token);

    if (c == '"')
        return scan_quoted(lexer, token, error, '"');
    if (c == '\'')
        return scan_quoted(lexer, token, error, '\'');

    return scan_operator(lexer, token, error);
}

/* ----------------------------------------------------------------- names */

#define NAME(k) case k: return #k

const char *lo_token_kind_name(lo_token_kind kind)
{
    switch (kind) {
    NAME(LO_LEXER_TOKEN_INVALID);
    NAME(LO_LEXER_TOKEN_EOF);
    NAME(LO_LEXER_TOKEN_IDENTIFIER);
    NAME(LO_LEXER_TOKEN_NUMBER);
    NAME(LO_LEXER_TOKEN_STRING);
    NAME(LO_LEXER_TOKEN_CHARACTER);
    NAME(LO_LEXER_TOKEN_KW_AUTO);
    NAME(LO_LEXER_TOKEN_KW_BREAK);
    NAME(LO_LEXER_TOKEN_KW_CASE);
    NAME(LO_LEXER_TOKEN_KW_CHAR);
    NAME(LO_LEXER_TOKEN_KW_CONST);
    NAME(LO_LEXER_TOKEN_KW_CONTINUE);
    NAME(LO_LEXER_TOKEN_KW_DEFAULT);
    NAME(LO_LEXER_TOKEN_KW_DO);
    NAME(LO_LEXER_TOKEN_KW_DOUBLE);
    NAME(LO_LEXER_TOKEN_KW_ELSE);
    NAME(LO_LEXER_TOKEN_KW_ENUM);
    NAME(LO_LEXER_TOKEN_KW_EXTERN);
    NAME(LO_LEXER_TOKEN_KW_FLOAT);
    NAME(LO_LEXER_TOKEN_KW_FOR);
    NAME(LO_LEXER_TOKEN_KW_GOTO);
    NAME(LO_LEXER_TOKEN_KW_IF);
    NAME(LO_LEXER_TOKEN_KW_INT);
    NAME(LO_LEXER_TOKEN_KW_LONG);
    NAME(LO_LEXER_TOKEN_KW_REGISTER);
    NAME(LO_LEXER_TOKEN_KW_RETURN);
    NAME(LO_LEXER_TOKEN_KW_SHORT);
    NAME(LO_LEXER_TOKEN_KW_SIGNED);
    NAME(LO_LEXER_TOKEN_KW_SIZEOF);
    NAME(LO_LEXER_TOKEN_KW_STATIC);
    NAME(LO_LEXER_TOKEN_KW_STRUCT);
    NAME(LO_LEXER_TOKEN_KW_SWITCH);
    NAME(LO_LEXER_TOKEN_KW_TYPEDEF);
    NAME(LO_LEXER_TOKEN_KW_UNION);
    NAME(LO_LEXER_TOKEN_KW_UNSIGNED);
    NAME(LO_LEXER_TOKEN_KW_VOID);
    NAME(LO_LEXER_TOKEN_KW_VOLATILE);
    NAME(LO_LEXER_TOKEN_KW_WHILE);
    NAME(LO_LEXER_TOKEN_PLUS);
    NAME(LO_LEXER_TOKEN_MINUS);
    NAME(LO_LEXER_TOKEN_STAR);
    NAME(LO_LEXER_TOKEN_SLASH);
    NAME(LO_LEXER_TOKEN_PERCENT);
    NAME(LO_LEXER_TOKEN_AMPERSAND);
    NAME(LO_LEXER_TOKEN_PIPE);
    NAME(LO_LEXER_TOKEN_CARET);
    NAME(LO_LEXER_TOKEN_TILDE);
    NAME(LO_LEXER_TOKEN_BANG);
    NAME(LO_LEXER_TOKEN_ASSIGN);
    NAME(LO_LEXER_TOKEN_EQUAL);
    NAME(LO_LEXER_TOKEN_NOT_EQUAL);
    NAME(LO_LEXER_TOKEN_LESS);
    NAME(LO_LEXER_TOKEN_LESS_EQUAL);
    NAME(LO_LEXER_TOKEN_GREATER);
    NAME(LO_LEXER_TOKEN_GREATER_EQUAL);
    NAME(LO_LEXER_TOKEN_INCREMENT);
    NAME(LO_LEXER_TOKEN_DECREMENT);
    NAME(LO_LEXER_TOKEN_AND);
    NAME(LO_LEXER_TOKEN_OR);
    NAME(LO_LEXER_TOKEN_LPAREN);
    NAME(LO_LEXER_TOKEN_RPAREN);
    NAME(LO_LEXER_TOKEN_LBRACE);
    NAME(LO_LEXER_TOKEN_RBRACE);
    NAME(LO_LEXER_TOKEN_LBRACKET);
    NAME(LO_LEXER_TOKEN_RBRACKET);
    NAME(LO_LEXER_TOKEN_COMMA);
    NAME(LO_LEXER_TOKEN_DOT);
    NAME(LO_LEXER_TOKEN_SEMICOLON);
    NAME(LO_LEXER_TOKEN_COLON);
    NAME(LO_LEXER_TOKEN_QUESTION);
    NAME(LO_LEXER_TOKEN_ARROW);
    }
    return "LO_LEXER_TOKEN_UNKNOWN";
}

const char *lo_lexer_error_name(lo_lexer_error_kind kind)
{
    switch (kind) {
    NAME(LO_LEXER_ERROR_NONE);
    NAME(LO_LEXER_ERROR_INVALID_CHARACTER);
    NAME(LO_LEXER_ERROR_UNTERMINATED_STRING);
    NAME(LO_LEXER_ERROR_UNTERMINATED_CHARACTER);
    NAME(LO_LEXER_ERROR_UNTERMINATED_COMMENT);
    }
    return "LO_LEXER_ERROR_UNKNOWN";
}

#undef NAME
