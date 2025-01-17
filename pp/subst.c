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

char *subst_tokens(const char *s, const defines *defs, const char *ignore_macro)
{
    str_t result = {0};
    token_state ts;
    set_token_string(&ts, s);

    const def *d = NULL;
    while (!TOKEN_STATE_DONE(&ts)) {
        const token t = get_token(&ts);
        if (ignore_macro && strcmp(ignore_macro, t.tok) == 0) {
            // ignore this macro (because we're already in it), replace the token as-is
            add_to_str(&result, t.tok);
        } else if ((d = defines_get(defs, t.tok))) {
            if (d->args) {
                handle_macro(d, defs, &ts, &result);
                // xxx after handling the macro, consider how to re-process again?
            } else {
                add_to_str(&result, d->replace);
            }
        } else if (strcmp(t.tok, "defined") == 0) {
            // special case: handle the `defined(SYMBOL)` pseudo-macro
            handle_defined(&ts, &result, defs);
        } else {
            add_to_str(&result, t.tok);
        }
    }
    return result.s;
}
