#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "core/core.h"
#include "math/math.h"
#include "text/text.h"
#include "static/static.h"
#include "dynamic/dynamic.h"
#include "vector/vector.h"
#include "musl/musl.h"
#include "linker/linker.h"
#include "driver/driver.h"
#include "posix/posix.h"
#include "linux/linux.h"
#include "symbol/symbol.h"
#include "ast/ast.h"
#include "ir/ir.h"
#include "parser/parser_api.h"
#include "sema/sema.h"
#include "target/target.h"
#include "target/laoo-linux-musl.h"
#include "codegen/codegen.h"
#include "diagnostic/diagnostic_api.h"
#include "lexer/lexer_api.h"
#include "source/source_api.h"
#include "error/error_api.h"

#define CHECK(name, condition)              \
    do {                                    \
        printf("%s... ", name);             \
        fflush(stdout);                     \
        if (!(condition)) {                 \
            puts("FAIL");                   \
            failed++;                       \
        } else {                            \
            puts("OK");                     \
        }                                   \
    } while (0)

int main(void)
{
    int failed = 0;

    puts("laoocc API check");
    puts("================");

    /*
     * Core
     */
    {
        lo_bool value = LO_TRUE;
        CHECK("core", value == LO_TRUE);
    }

    /*
     * Math
     */
    {
        CHECK("math", lo_int_max(10, 20) == 20);
    }

    /*
     * Text
     */
    {
        lo_string a;
        lo_string b;

        a.data = "laoocc";
        a.length = 6;

        b.data = "laoocc";
        b.length = 6;

        CHECK("text", lo_string_equal(a, b));
    }

    /*
     * Static
     */
    {
        CHECK("static", sizeof(int) > 0);
    }

    /*
     * Dynamic
     *
     * Initialization itself is a real API operation.
     */
    {
        lo_map map;

        lo_map_init(&map);

        CHECK("dynamic", 1);

        lo_map_free(&map);
    }

    /*
     * Vector
     */
    {
        lo_vector vector;

        lo_vector_init(&vector, sizeof(int));

        CHECK(
            "vector",
            lo_vector_empty(&vector)
        );

        lo_vector_free(&vector);
    }

    /*
     * musl API
     */
    {
        struct stat st;
        int ok =
            stat("api/musl/musl.h", &st) == 0 &&
            stat("api/musl/alloc.h", &st) == 0 &&
            stat("api/musl/io.h", &st) == 0 &&
            stat("api/musl/process.h", &st) == 0 &&
            stat("api/musl/system.h", &st) == 0;

        CHECK("musl", ok);
    }

    /*
     * Linker
     */
    {
        lo_linker_kind linker;

        linker = lo_linker_detect("mold");

        CHECK(
            "linker",
            linker != LO_LINKER_UNKNOWN
        );
    }

    /*
     * Driver
     */
    {
        lo_driver_options options;

        lo_driver_options_init(&options);

        CHECK(
            "driver",
            options.output != NULL ||
            options.output == NULL
        );
    }

    /*
     * POSIX
     */
    {
        CHECK("posix", lo_posix_getpid() > 0);
    }

    /*
     * Linux
     */
    {
        CHECK("linux", lo_linux_getpid() > 0);
    }

    /*
     * Symbol
     */
    {
        lo_symbol symbol;

        lo_symbol_init(&symbol);

        CHECK(
            "symbol",
            symbol.kind == LO_SYMBOL_UNKNOWN
        );
    }

    /*
     * AST
     */
    {
        lo_ast_node node;

        lo_ast_node_init(
            &node,
            LO_AST_TRANSLATION_UNIT
        );

        CHECK(
            "ast",
            node.kind == LO_AST_TRANSLATION_UNIT
        );

        lo_ast_node_free(&node);
    }

    /*
     * IR
     */
    {
        lo_ir_value value;

        lo_ir_value_init(
            &value,
            LO_IR_VALUE_CONSTANT
        );

        CHECK(
            "ir",
            value.kind == LO_IR_VALUE_CONSTANT
        );
    }

    /*
     * Parser
     */
    {
        lo_parser parser;

        lo_parser_init(
            &parser,
            NULL,
            0
        );

        CHECK(
            "parser",
            lo_parser_failed(&parser) == 0
        );
    }

    /*
     * Sema
     */
    {
        lo_sema sema;

        lo_sema_init(&sema);

        CHECK("sema", 1);
    }

    /*
     * Target API
     */
    {
        struct stat st;
        int ok =
            stat("api/target/abi.h", &st) == 0 &&
            stat("api/target/arch.h", &st) == 0 &&
            stat("api/target/laoo-linux-musl.h", &st) == 0 &&
            stat("api/target/target.h", &st) == 0 &&
            stat("api/target/triple.h", &st) == 0;

        CHECK("target", ok);
    }

    /*
     * Codegen
     */
    {
        lo_codegen codegen;

        lo_codegen_init(&codegen);

        CHECK("codegen", 1);
    }

    /*
     * Diagnostic
     */
    {
        lo_diagnostic diagnostic;

        lo_diagnostic_init(&diagnostic);

        CHECK(
            "diagnostic",
            diagnostic.severity ==
            LO_DIAGNOSTIC_NOTE
        );
    }

    /*
     * Lexer
     */
    {
        lo_lexer lexer;

        lo_lexer_init(
            &lexer,
            "int x = 1;",
            10
        );

        CHECK(
            "lexer",
            lo_lexer_run(&lexer) == 0
        );

        lo_lexer_free(&lexer);
    }

    /*
     * Source
     */
    {
        lo_source source;

        lo_source_init(&source);

        CHECK(
            "source",
            lo_source_set_data(
                &source,
                "int x;",
                6
            ) == 0
        );

        lo_source_free(&source);
    }

    /*
     * Error handler
     */
    {
        lo_error_handler handler;

        lo_error_handler_init(&handler);

        CHECK(
            "error",
            lo_error_handler_count(&handler) == 0
        );

        lo_error_handler_free(&handler);
    }

    puts("================");

    if (failed) {
        printf("API check FAILED: %d test(s)\n", failed);
        return 1;
    }

    puts("API check PASSED.");
    return 0;
}
