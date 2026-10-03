/*
 * laoocc preprocessor v1 - alternative implementation file.
 *
 * Implements every function declared in preprocessor.h / directive.h:
 *   lo_preprocessor_init, lo_preprocessor_run, lo_preprocessor_free,
 *   lo_pp_error_name, lo_pp_directive_name
 * Build it INSTEAD of the stub preprocessor.c (not together with it).
 *
 * Supported: object-like #define, #undef, #ifdef, #ifndef, #if (integer
 * literal / defined), #else, #endif, #include "file", recursive macro
 * expansion with self-reference protection, comment- and string-aware
 * scanning, backslash-newline in directives.
 *
 * Ownership: each lo_pp_macro owns ONE heap block that holds
 * "name\0replacement\0"; macro->name is the block base pointer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocessor_api.h"

#define PP_MAX_EXPAND_DEPTH 64
#define PP_MAX_INCLUDE_DEPTH 32
#define PP_NOT_FOUND ((size_t)-1)

/* Optional extra include directory (pointer must outlive the run). */
static const char *g_include_dir = NULL;

void lo_pp_set_include_dir(const char *dir)
{
    g_include_dir = dir;
}

/* ------------------------------------------------------------------ */
/* names                                                               */
/* ------------------------------------------------------------------ */

const char *lo_pp_error_name(lo_pp_error_kind kind)
{
    switch (kind) {
    case LO_PP_ERROR_NONE:                    return "none";
    case LO_PP_ERROR_INVALID_DIRECTIVE:       return "invalid directive";
    case LO_PP_ERROR_INVALID_MACRO:           return "invalid macro";
    case LO_PP_ERROR_UNDEFINED_MACRO:         return "undefined macro";
    case LO_PP_ERROR_UNTERMINATED_CONDITIONAL:return "unterminated conditional";
    case LO_PP_ERROR_UNEXPECTED_ELSE:         return "unexpected #else";
    case LO_PP_ERROR_UNEXPECTED_ENDIF:        return "unexpected #endif";
    case LO_PP_ERROR_INCLUDE_NOT_FOUND:       return "include not found";
    }
    return "unknown";
}

const char *lo_pp_directive_name(lo_pp_directive_kind kind)
{
    switch (kind) {
    case LO_PP_DIRECTIVE_UNKNOWN: return "unknown";
    case LO_PP_DIRECTIVE_DEFINE:  return "define";
    case LO_PP_DIRECTIVE_UNDEF:   return "undef";
    case LO_PP_DIRECTIVE_INCLUDE: return "include";
    case LO_PP_DIRECTIVE_IF:      return "if";
    case LO_PP_DIRECTIVE_IFDEF:   return "ifdef";
    case LO_PP_DIRECTIVE_IFNDEF:  return "ifndef";
    case LO_PP_DIRECTIVE_ELSE:    return "else";
    case LO_PP_DIRECTIVE_ENDIF:   return "endif";
    }
    return "unknown";
}

/* ------------------------------------------------------------------ */
/* small growable byte buffer                                          */
/* ------------------------------------------------------------------ */

typedef struct pp_buf {
    char *data;
    size_t length;
    size_t capacity;
} pp_buf;

static int buf_reserve(pp_buf *b, size_t extra)
{
    size_t need, cap;
    char *p;

    if (extra > SIZE_MAX - b->length - 1) {
        return -1;
    }
    need = b->length + extra + 1;
    if (need <= b->capacity) {
        return 0;
    }
    cap = b->capacity ? b->capacity : 256;
    while (cap < need) {
        if (cap > SIZE_MAX / 2) {
            return -1;
        }
        cap *= 2;
    }
    p = realloc(b->data, cap);
    if (!p) {
        return -1;
    }
    b->data = p;
    b->capacity = cap;
    return 0;
}

static int buf_append(pp_buf *b, const char *s, size_t n)
{
    if (n == 0) {
        return 0;
    }
    if (buf_reserve(b, n) != 0) {
        return -1;
    }
    memcpy(b->data + b->length, s, n);
    b->length += n;
    b->data[b->length] = '\0';
    return 0;
}

/* ------------------------------------------------------------------ */
/* character classes (ASCII only, locale independent)                  */
/* ------------------------------------------------------------------ */

static int is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static int is_id_start(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_id_char(char c)
{
    return is_id_start(c) || is_digit(c);
}

/* ------------------------------------------------------------------ */
/* context + errors                                                    */
/* ------------------------------------------------------------------ */

typedef struct pp_ctx {
    lo_preprocessor *pp;
    pp_buf *out;
    lo_pp_error *err;
    int depth;
} pp_ctx;

static int pp_fail(pp_ctx *c, lo_pp_error_kind kind, const char *msg,
                   size_t line, size_t col)
{
    if (c->err) {
        c->err->kind = kind;
        c->err->message = msg;
        c->err->line = line;
        c->err->column = col;
    }
    return -1;
}

/* The API has no out-of-memory error kind; INVALID_DIRECTIVE + message. */
static int pp_oom(pp_ctx *c)
{
    return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, "out of memory",
                   c->pp->line, 1);
}

/* ------------------------------------------------------------------ */
/* macro table (linear)                                                */
/* ------------------------------------------------------------------ */

static size_t macro_find(const lo_preprocessor *pp, const char *name, size_t n)
{
    size_t i;

    for (i = 0; i < pp->macro_count; i++) {
        const lo_pp_macro *m = &pp->macros[i];
        if (m->name_length == n && memcmp(m->name, name, n) == 0) {
            return i;
        }
    }
    return PP_NOT_FOUND;
}

static int macro_set(lo_preprocessor *pp, const char *name, size_t nl,
                     const char *repl, size_t rl)
{
    size_t idx;
    char *block;
    lo_pp_macro *m;

    if (nl > SIZE_MAX - rl - 2) {
        return -1;
    }
    block = malloc(nl + rl + 2);
    if (!block) {
        return -1;
    }
    memcpy(block, name, nl);
    block[nl] = '\0';
    memcpy(block + nl + 1, repl, rl);
    block[nl + 1 + rl] = '\0';

    idx = macro_find(pp, name, nl);
    if (idx == PP_NOT_FOUND) {
        if (pp->macro_count == pp->macro_capacity) {
            size_t cap = pp->macro_capacity ? pp->macro_capacity * 2 : 16;
            lo_pp_macro *grown = realloc(pp->macros, cap * sizeof *grown);
            if (!grown) {
                free(block);
                return -1;
            }
            pp->macros = grown;
            pp->macro_capacity = cap;
        }
        idx = pp->macro_count++;
    } else {
        free((void *)pp->macros[idx].name);
    }
    m = &pp->macros[idx];
    m->name = block;
    m->name_length = nl;
    m->replacement = block + nl + 1;
    m->replacement_length = rl;
    return 0;
}

static void macro_remove(lo_preprocessor *pp, size_t idx)
{
    free((void *)pp->macros[idx].name);
    memmove(&pp->macros[idx], &pp->macros[idx + 1],
            (pp->macro_count - idx - 1) * sizeof pp->macros[0]);
    pp->macro_count--;
}

/* ------------------------------------------------------------------ */
/* conditional stack                                                   */
/* ------------------------------------------------------------------ */

static int is_active(const lo_preprocessor *pp)
{
    return pp->conditional_count == 0
        || pp->conditionals[pp->conditional_count - 1].active;
}

static int cond_push(lo_preprocessor *pp, int parent_active, int cond)
{
    lo_pp_conditional *cd;

    if (pp->conditional_count == pp->conditional_capacity) {
        size_t cap = pp->conditional_capacity ? pp->conditional_capacity * 2 : 8;
        lo_pp_conditional *grown = realloc(pp->conditionals, cap * sizeof *grown);
        if (!grown) {
            return -1;
        }
        pp->conditionals = grown;
        pp->conditional_capacity = cap;
    }
    cd = &pp->conditionals[pp->conditional_count++];
    cd->parent_active = parent_active;
    cd->active = parent_active && cond;
    cd->else_seen = 0;
    return 0;
}

/* ------------------------------------------------------------------ */
/* macro expansion of ordinary source text                             */
/* ------------------------------------------------------------------ */

static int expand_text(pp_ctx *c, const char *s, size_t n, int *in_block,
                       size_t *hide, size_t hide_count)
{
    size_t i = 0;

    while (i < n) {
        char ch = s[i];
        size_t st;

        if (*in_block) {
            st = i;
            while (i < n && !(s[i] == '*' && i + 1 < n && s[i + 1] == '/')) {
                i++;
            }
            if (i < n) {
                i += 2;
                *in_block = 0;
            }
            if (buf_append(c->out, s + st, i - st)) return pp_oom(c);
            continue;
        }

        if (ch == '/' && i + 1 < n && s[i + 1] == '/') {
            if (buf_append(c->out, s + i, n - i)) return pp_oom(c);
            return 0;
        }
        if (ch == '/' && i + 1 < n && s[i + 1] == '*') {
            *in_block = 1;
            if (buf_append(c->out, "/*", 2)) return pp_oom(c);
            i += 2;
            continue;
        }
        if (ch == '"' || ch == '\'') {
            st = i++;
            while (i < n && s[i] != ch) {
                if (s[i] == '\\' && i + 1 < n) i++;
                i++;
            }
            if (i < n) i++;
            if (buf_append(c->out, s + st, i - st)) return pp_oom(c);
            continue;
        }
        if (is_digit(ch)) { /* pp-number: never expand inside */
            st = i++;
            while (i < n) {
                char p = s[i];
                if (is_id_char(p) || p == '.') {
                    i++;
                } else if ((p == '+' || p == '-')
                           && (s[i - 1] == 'e' || s[i - 1] == 'E'
                               || s[i - 1] == 'p' || s[i - 1] == 'P')) {
                    i++;
                } else {
                    break;
                }
            }
            if (buf_append(c->out, s + st, i - st)) return pp_oom(c);
            continue;
        }
        if (is_id_start(ch)) {
            size_t idx, k;
            int hidden = 0;

            st = i;
            while (i < n && is_id_char(s[i])) i++;
            idx = macro_find(c->pp, s + st, i - st);
            if (idx != PP_NOT_FOUND) {
                for (k = 0; k < hide_count; k++) {
                    if (hide[k] == idx) { hidden = 1; break; }
                }
            }
            if (idx != PP_NOT_FOUND && !hidden && hide_count < PP_MAX_EXPAND_DEPTH) {
                const lo_pp_macro *m = &c->pp->macros[idx];
                int local_block = 0;
                hide[hide_count] = idx;
                if (expand_text(c, m->replacement, m->replacement_length,
                                &local_block, hide, hide_count + 1)) {
                    return -1;
                }
            } else if (buf_append(c->out, s + st, i - st)) {
                return pp_oom(c);
            }
            continue;
        }

        /* run of plain characters */
        st = i;
        while (i < n) {
            char p = s[i];
            if (is_id_char(p) || p == '"' || p == '\'' || p == '/') break;
            i++;
        }
        if (i == st) i++; /* lone '/' */
        if (buf_append(c->out, s + st, i - st)) return pp_oom(c);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* directive text cleaning: strip comments + line continuations        */
/* dst may be NULL (state tracking only). Returns cleaned length.      */
/* ------------------------------------------------------------------ */

static size_t clean_text(const char *s, size_t n, char *dst, int *in_block)
{
    size_t i = 0, o = 0;

    while (i < n) {
        char ch = s[i];

        if (*in_block) {
            if (ch == '*' && i + 1 < n && s[i + 1] == '/') {
                *in_block = 0;
                i += 2;
                if (dst) dst[o] = ' ';
                o++;
            } else {
                i++;
            }
            continue;
        }
        if (ch == '\\') {
            size_t j = i + 1;
            if (j < n && s[j] == '\r') j++;
            if (j < n && s[j] == '\n') {
                i = j + 1;
                continue;
            }
        }
        if (ch == '/' && i + 1 < n && s[i + 1] == '/') {
            break;
        }
        if (ch == '/' && i + 1 < n && s[i + 1] == '*') {
            *in_block = 1;
            i += 2;
            continue;
        }
        if (ch == '"' || ch == '\'') {
            if (dst) dst[o] = ch;
            o++;
            i++;
            while (i < n && s[i] != ch) {
                if (s[i] == '\\' && i + 1 < n) {
                    if (dst) dst[o] = s[i];
                    o++;
                    i++;
                }
                if (dst) dst[o] = s[i];
                o++;
                i++;
            }
            if (i < n) {
                if (dst) dst[o] = s[i];
                o++;
                i++;
            }
            continue;
        }
        if (dst) dst[o] = ch;
        o++;
        i++;
    }
    return o;
}

/* ------------------------------------------------------------------ */
/* directive helpers                                                   */
/* ------------------------------------------------------------------ */

static void skip_ws(const char *t, size_t n, size_t *p)
{
    while (*p < n && is_ws(t[*p])) (*p)++;
}

static int parse_ident(const char *t, size_t n, size_t *p, size_t *start,
                       size_t *len)
{
    skip_ws(t, n, p);
    if (*p >= n || !is_id_start(t[*p])) return 0;
    *start = *p;
    while (*p < n && is_id_char(t[*p])) (*p)++;
    *len = *p - *start;
    return 1;
}

static lo_pp_directive_kind classify(const char *s, size_t n)
{
    struct { const char *name; size_t len; lo_pp_directive_kind kind; } tbl[] = {
        { "define", 6, LO_PP_DIRECTIVE_DEFINE },
        { "undef",  5, LO_PP_DIRECTIVE_UNDEF },
        { "include",7, LO_PP_DIRECTIVE_INCLUDE },
        { "ifdef",  5, LO_PP_DIRECTIVE_IFDEF },
        { "ifndef", 6, LO_PP_DIRECTIVE_IFNDEF },
        { "if",     2, LO_PP_DIRECTIVE_IF },
        { "else",   4, LO_PP_DIRECTIVE_ELSE },
        { "endif",  5, LO_PP_DIRECTIVE_ENDIF }
    };
    size_t i;

    for (i = 0; i < sizeof tbl / sizeof tbl[0]; i++) {
        if (tbl[i].len == n && memcmp(tbl[i].name, s, n) == 0) {
            return tbl[i].kind;
        }
    }
    return LO_PP_DIRECTIVE_UNKNOWN;
}

/* #if: integer literal, "defined X", "defined(X)", optional leading '!' */
static int eval_if(pp_ctx *c, const char *t, size_t n, size_t p,
                   int *result, size_t line, size_t col)
{
    int negate = 0;
    const char *bad = "unsupported #if expression";

    skip_ws(t, n, &p);
    while (p < n && t[p] == '!') {
        negate = !negate;
        p++;
        skip_ws(t, n, &p);
    }
    if (p < n && is_digit(t[p])) {
        char *end;
        long v = strtol(t + p, &end, 0); /* t is NUL terminated */
        p = (size_t)(end - t);
        skip_ws(t, n, &p);
        if (p != n) return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, bad, line, col);
        *result = (v != 0) != negate;
        return 0;
    }
    if (n - p >= 7 && memcmp(t + p, "defined", 7) == 0 && !is_id_char(t[p + 7])) {
        size_t s, l, q = p + 7;
        int paren = 0;
        skip_ws(t, n, &q);
        if (q < n && t[q] == '(') { paren = 1; q++; }
        if (!parse_ident(t, n, &q, &s, &l)) {
            return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, bad, line, col);
        }
        skip_ws(t, n, &q);
        if (paren) {
            if (q >= n || t[q] != ')') {
                return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, bad, line, col);
            }
            q++;
            skip_ws(t, n, &q);
        }
        if (q != n) return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, bad, line, col);
        *result = (macro_find(c->pp, t + s, l) != PP_NOT_FOUND) != negate;
        return 0;
    }
    return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE, bad, line, col);
}

/* ------------------------------------------------------------------ */
/* include                                                             */
/* ------------------------------------------------------------------ */

static int read_file(const char *path, pp_buf *dst)
{
    FILE *f = fopen(path, "rb");
    char chunk[4096];
    size_t got;

    if (!f) return -1;
    while ((got = fread(chunk, 1, sizeof chunk, f)) > 0) {
        if (buf_append(dst, chunk, got)) { fclose(f); return -2; }
    }
    if (ferror(f)) { fclose(f); return -1; }
    fclose(f);
    if (!dst->data && buf_reserve(dst, 0) == 0) dst->data[0] = '\0';
    return 0;
}

static int process(pp_ctx *c, const char *src, size_t len, const char *dir);

static int do_include(pp_ctx *c, const char *t, size_t n, size_t p,
                      size_t line, size_t col, const char *dir)
{
    char close;
    size_t ns, nl, i;
    const char *roots[3];
    pp_buf file = { NULL, 0, 0 };
    char *path = NULL, *newdir = NULL;
    int rc;

    skip_ws(t, n, &p);
    if (p >= n || (t[p] != '"' && t[p] != '<')) {
        return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE,
                       "expected \"file\" after #include", line, col);
    }
    close = t[p] == '"' ? '"' : '>';
    ns = ++p;
    while (p < n && t[p] != close) p++;
    if (p >= n || p == ns) {
        return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE,
                       "malformed #include", line, col);
    }
    nl = p - ns;

    if (c->depth >= PP_MAX_INCLUDE_DEPTH) {
        return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE,
                       "#include nested too deeply", line, col);
    }

    roots[0] = dir;
    roots[1] = g_include_dir;
    roots[2] = NULL;
    for (i = 0; i < 3; i++) {
        size_t rl = roots[i] ? strlen(roots[i]) : 0;
        int r;

        if (i < 2 && rl == 0) continue;
        path = malloc(rl + 1 + nl + 1);
        if (!path) return pp_oom(c);
        if (rl) {
            memcpy(path, roots[i], rl);
            path[rl] = '/';
            memcpy(path + rl + 1, t + ns, nl);
            path[rl + 1 + nl] = '\0';
        } else {
            memcpy(path, t + ns, nl);
            path[nl] = '\0';
        }
        r = read_file(path, &file);
        if (r == -2) { free(path); free(file.data); return pp_oom(c); }
        if (r == 0) break;
        free(file.data);
        file.data = NULL; file.length = file.capacity = 0;
        free(path);
        path = NULL;
    }
    if (!path) {
        return pp_fail(c, LO_PP_ERROR_INCLUDE_NOT_FOUND,
                       "include file not found", line, col);
    }

    {
        const char *slash = strrchr(path, '/');
        size_t dl = slash ? (size_t)(slash - path) : 0;
        if (slash) {
            newdir = malloc(dl + 1);
            if (!newdir) { free(path); free(file.data); return pp_oom(c); }
            memcpy(newdir, path, dl);
            newdir[dl] = '\0';
        }
    }
    free(path);

    c->depth++;
    rc = process(c, file.data ? file.data : "", file.length, newdir);
    c->depth--;
    free(newdir);
    free(file.data);
    if (rc) return rc;

    if (c->out->length > 0 && c->out->data[c->out->length - 1] != '\n') {
        if (buf_append(c->out, "\n", 1)) return pp_oom(c);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* directive dispatch                                                  */
/* ------------------------------------------------------------------ */

static int run_directive(pp_ctx *c, const char *t, size_t n, size_t line,
                         size_t col, const char *dir)
{
    lo_preprocessor *pp = c->pp;
    size_t p = 0, ks, kl, ns, nl;
    int active = is_active(pp);
    lo_pp_directive_kind kind;

    skip_ws(t, n, &p);
    ks = p;
    while (p < n && is_id_char(t[p])) p++;
    kl = p - ks;
    if (kl == 0) {
        skip_ws(t, n, &p);
        if (p >= n || !active) return 0; /* null directive */
        return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE,
                       "invalid preprocessing directive", line, col);
    }
    kind = classify(t + ks, kl);

    switch (kind) {
    case LO_PP_DIRECTIVE_IFDEF:
    case LO_PP_DIRECTIVE_IFNDEF: {
        int cond = 0;
        if (active) {
            if (!parse_ident(t, n, &p, &ns, &nl)) {
                return pp_fail(c, LO_PP_ERROR_INVALID_MACRO,
                               "expected macro name", line, col);
            }
            cond = macro_find(pp, t + ns, nl) != PP_NOT_FOUND;
            if (kind == LO_PP_DIRECTIVE_IFNDEF) cond = !cond;
        }
        if (cond_push(pp, active, cond)) return pp_oom(c);
        return 0;
    }
    case LO_PP_DIRECTIVE_IF: {
        int cond = 0;
        if (active && eval_if(c, t, n, p, &cond, line, col)) return -1;
        if (cond_push(pp, active, cond)) return pp_oom(c);
        return 0;
    }
    case LO_PP_DIRECTIVE_ELSE: {
        lo_pp_conditional *cd;
        /* underflow vs. the current file's base is checked in process() */
        cd = &pp->conditionals[pp->conditional_count - 1];
        if (cd->else_seen) {
            return pp_fail(c, LO_PP_ERROR_UNEXPECTED_ELSE,
                           "duplicate #else", line, col);
        }
        cd->else_seen = 1;
        cd->active = cd->parent_active && !cd->active;
        return 0;
    }
    case LO_PP_DIRECTIVE_ENDIF:
        pp->conditional_count--;
        return 0;
    case LO_PP_DIRECTIVE_DEFINE: {
        size_t re;
        if (!active) return 0;
        if (!parse_ident(t, n, &p, &ns, &nl)) {
            return pp_fail(c, LO_PP_ERROR_INVALID_MACRO,
                           "expected macro name after #define", line, col);
        }
        if (p < n && t[p] == '(') {
            return pp_fail(c, LO_PP_ERROR_INVALID_MACRO,
                           "function-like macros are not supported", line, col);
        }
        skip_ws(t, n, &p);
        re = n;
        while (re > p && is_ws(t[re - 1])) re--;
        if (macro_set(pp, t + ns, nl, t + p, re - p)) return pp_oom(c);
        return 0;
    }
    case LO_PP_DIRECTIVE_UNDEF: {
        size_t idx;
        if (!active) return 0;
        if (!parse_ident(t, n, &p, &ns, &nl)) {
            return pp_fail(c, LO_PP_ERROR_INVALID_MACRO,
                           "expected macro name after #undef", line, col);
        }
        idx = macro_find(pp, t + ns, nl);
        if (idx != PP_NOT_FOUND) macro_remove(pp, idx);
        return 0;
    }
    case LO_PP_DIRECTIVE_INCLUDE:
        if (!active) return 0;
        return do_include(c, t, n, p, line, col, dir);
    case LO_PP_DIRECTIVE_UNKNOWN:
    default:
        if (!active) return 0;
        return pp_fail(c, LO_PP_ERROR_INVALID_DIRECTIVE,
                       "unknown preprocessing directive", line, col);
    }
}

/* ------------------------------------------------------------------ */
/* line-by-line driver                                                 */
/* ------------------------------------------------------------------ */

/* End of the logical line starting at pos (index of '\n' or len). */
static size_t logical_line_end(const char *s, size_t len, size_t pos,
                               size_t *lines)
{
    size_t i = pos;

    *lines = 1;
    while (i < len) {
        if (s[i] == '\n') {
            size_t b = i;
            if (b > pos && s[b - 1] == '\r') b--;
            if (b > pos && s[b - 1] == '\\') {
                (*lines)++;
                i++;
                continue;
            }
            return i;
        }
        i++;
    }
    return len;
}

static int emit_newlines(pp_ctx *c, size_t count)
{
    while (count--) {
        if (buf_append(c->out, "\n", 1)) return pp_oom(c);
    }
    return 0;
}

static int process(pp_ctx *c, const char *src, size_t len, const char *dir)
{
    lo_preprocessor *pp = c->pp;
    size_t base = pp->conditional_count;
    size_t saved_line = pp->line;
    size_t pos = 0;
    int in_block = 0;
    int rc = 0;
    size_t hide[PP_MAX_EXPAND_DEPTH];

    pp->line = 1;

    while (pos < len) {
        size_t lines, end, line_len, hash, nls;
        int is_dir = 0;

        end = logical_line_end(src, len, pos, &lines);
        line_len = end - pos;
        nls = (lines - 1) + (end < len ? 1 : 0);

        hash = pos;
        if (!in_block) {
            while (hash < end && (src[hash] == ' ' || src[hash] == '\t'
                                  || src[hash] == '\v' || src[hash] == '\f')) {
                hash++;
            }
            is_dir = hash < end && src[hash] == '#';
        }

        if (is_dir) {
            size_t col = hash - pos + 1;
            const char *text = src + hash + 1;
            size_t tn = end - (hash + 1);
            char *tmp = malloc(tn + 1);
            size_t tl;
            lo_pp_directive_kind peek;

            if (!tmp) { rc = pp_oom(c); goto done; }
            tl = clean_text(text, tn, tmp, &in_block);
            tmp[tl] = '\0';

            {   /* conditional-stack underflow is relative to this file */
                size_t k = 0, s0;
                skip_ws(tmp, tl, &k);
                s0 = k;
                while (k < tl && is_id_char(tmp[k])) k++;
                peek = classify(tmp + s0, k - s0);
            }
            if (peek == LO_PP_DIRECTIVE_ELSE && pp->conditional_count <= base) {
                rc = pp_fail(c, LO_PP_ERROR_UNEXPECTED_ELSE,
                             "#else without matching #if", pp->line, col);
            } else if (peek == LO_PP_DIRECTIVE_ENDIF
                       && pp->conditional_count <= base) {
                rc = pp_fail(c, LO_PP_ERROR_UNEXPECTED_ENDIF,
                             "#endif without matching #if", pp->line, col);
            } else {
                rc = run_directive(c, tmp, tl, pp->line, col, dir);
            }
            free(tmp);
            if (rc) goto done;
            if (in_block && is_active(pp)) {
                if (buf_append(c->out, "/*", 2)) { rc = pp_oom(c); goto done; }
            }
            if (emit_newlines(c, nls)) { rc = -1; goto done; }
        } else if (!is_active(pp)) {
            (void)clean_text(src + pos, line_len, NULL, &in_block);
            if (emit_newlines(c, nls)) { rc = -1; goto done; }
        } else {
            if (expand_text(c, src + pos, line_len, &in_block, hide, 0)) {
                rc = -1;
                goto done;
            }
            if (end < len && buf_append(c->out, "\n", 1)) {
                rc = pp_oom(c);
                goto done;
            }
        }

        pp->line += lines;
        pos = end < len ? end + 1 : len;
    }

    if (pp->conditional_count > base) {
        rc = pp_fail(c, LO_PP_ERROR_UNTERMINATED_CONDITIONAL,
                     "unterminated conditional directive", pp->line, 1);
    }

done:
    if (rc == 0) pp->line = saved_line;
    return rc;
}

/* ------------------------------------------------------------------ */
/* public API                                                          */
/* ------------------------------------------------------------------ */

void lo_preprocessor_init(lo_preprocessor *preprocessor, const char *source,
                          size_t length)
{
    if (!preprocessor) return;
    memset(preprocessor, 0, sizeof *preprocessor);
    preprocessor->source = source;
    preprocessor->length = length;
    preprocessor->line = 1;
    preprocessor->column = 1;
}

int lo_preprocessor_run(lo_preprocessor *preprocessor, char **output,
                        size_t *output_length, lo_pp_error *error)
{
    pp_buf out = { NULL, 0, 0 };
    pp_ctx ctx;
    int rc;

    if (output) *output = NULL;
    if (output_length) *output_length = 0;
    if (error) {
        error->kind = LO_PP_ERROR_NONE;
        error->message = NULL;
        error->line = 0;
        error->column = 0;
    }
    if (!preprocessor || !output || !output_length) {
        if (error) {
            error->kind = LO_PP_ERROR_INVALID_DIRECTIVE;
            error->message = "invalid arguments";
        }
        return -1;
    }

    preprocessor->offset = 0;
    preprocessor->line = 1;
    preprocessor->column = 1;
    preprocessor->conditional_count = 0;

    ctx.pp = preprocessor;
    ctx.out = &out;
    ctx.err = error;
    ctx.depth = 0;

    if (buf_reserve(&out, preprocessor->length) != 0) {
        return pp_oom(&ctx);
    }
    out.data[0] = '\0';

    rc = process(&ctx, preprocessor->source ? preprocessor->source : "",
                 preprocessor->source ? preprocessor->length : 0, NULL);
    if (rc != 0) {
        free(out.data);
        return -1;
    }

    preprocessor->offset = preprocessor->length;
    *output = out.data;
    *output_length = out.length;
    return 0;
}

void lo_preprocessor_free(lo_preprocessor *preprocessor)
{
    size_t i;

    if (!preprocessor) return;
    for (i = 0; i < preprocessor->macro_count; i++) {
        free((void *)preprocessor->macros[i].name);
    }
    free(preprocessor->macros);
    free(preprocessor->conditionals);
    preprocessor->macros = NULL;
    preprocessor->macro_count = 0;
    preprocessor->macro_capacity = 0;
    preprocessor->conditionals = NULL;
    preprocessor->conditional_count = 0;
    preprocessor->conditional_capacity = 0;
}
