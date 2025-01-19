#include <string.h>
#include "pp_macro.h"
#include "lex.h"
#include "str.h"
#include "die.h"

static void handle_defined(token_state *ts, str_t *out, const defines *defs)
{
    int found_parens = 0;

    while (!TOKEN_STATE_DONE(ts)) {
        const token t = get_token(ts);
        if (t.type == TOK_WS) {
            continue;
        }
        if (t.type == '(') {
            found_parens = 1;
            continue;
        }
        const int found = defines_get(defs, t.tok) != NULL;
        add_to_str(out, found ? "1" : "0");
        break;
    }

    while (found_parens && !TOKEN_STATE_DONE(ts)) {
        const token t = get_token(ts);
        if (t.type == TOK_WS) {
            continue;
        }
        if (t.type == ')') {
            found_parens = 0;
        }
    }

    if (found_parens) {
        die("Unexpected %s while looking for closing parens",
            TOKEN_STATE_DONE(ts) ? "end-of-line" : ts->last.tok);
    }
}

// ReSharper disable CppDFADeletedPointer

char *subst_tokens(const char *s, const defines *defs)
{
    str_t result = {0};
    token_state ts;
    set_token_string(&ts, s);
    ignore_list ignored = {0};

    if (!s) {
        return NULL;
    }

    def *d = NULL;
    while (!TOKEN_STATE_DONE(&ts)) {

        // un-ignore all ignored defines once we're no longer reading the macro expansion result
        if (!TOKEN_STATE_READING_IGNORED(&ts) && ignored.count) {
            clear_ignore_list(&ignored);
        }

        const token t = get_token(&ts);
        if ((d = defines_get(defs, t.tok))) {

            str_t macro_result = {0};
            if (handle_macro(d, defs, &ts, &macro_result)) {
                // ignore this definition until we are done re-parsing the result
                // push the macro expansion back into the token string for re-parsing
                add_to_ignore_list(&ignored, d);
                push_back_token_data(&ts, macro_result.s);
            } else {
                add_to_str(&result, macro_result.s);
            }
            free_str(&macro_result);
        } else if (strcmp(t.tok, "defined") == 0) {
            // special case: handle the `defined(SYMBOL)` pseudo-macro
            handle_defined(&ts, &result, defs);
        } else {
            add_to_str(&result, t.tok);
        }
    }

    if (ignored.count) {
        clear_ignore_list(&ignored);
    }
    return result.s;
}
