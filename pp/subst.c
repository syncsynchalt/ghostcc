#include <string.h>
#include <stdio.h>
#include "pp_macro.h"
#include "lex.h"
#include "str.h"
#include "die.h"

static void handle_defined(token_state *ts, str_t *out, const defines *defs)
{
    int found_parens = 0;

    while (!LINE_DONE(ts, ts->_line)) {
        get_token(ts->_line, ts->_line_len, ts);
        if (ts->type == TOK_WS) {
            continue;
        }
        if (ts->type == '(') {
            found_parens = 1;
            continue;
        }
        const int found = defines_get(defs, ts->tok) != NULL;
        add_to_str(out, found ? "1" : "0");
        break;
    }

    while (found_parens && !LINE_DONE(ts, ts->_line)) {
        get_token(ts->_line, ts->_line_len, ts);
        if (ts->type == TOK_WS) {
            continue;
        }
        if (ts->type == ')') {
            found_parens = 0;
            continue;
        }
        break;
    }

    if (found_parens) {
        die("Unexpected %s while looking for closing parens",
            LINE_DONE(ts, ts->_line) ? "end-of-line" : ts->tok);
    }
}

char *subst_tokens(const char *s, const defines *defs, const char *ignore_macro)
{
    token_state ts = {0};
    const size_t slen = strlen(s);
    str_t result = {0};

    const def *d = NULL;
    while (!LINE_DONE(&ts, s)) {
        get_token(s, slen, &ts);
        if (ignore_macro && strcmp(ignore_macro, ts.tok) == 0) {
            // ignore this macro (because we're already in it), replace the token as-is
            add_to_str(&result, ts.tok);
        } else if ((d = defines_get(defs, ts.tok))) {
            if (d->args) {
                handle_macro(d, defs, &ts, &result);
                // xxx after handling the macro, consider how to re-process again?
            } else {
                add_to_str(&result, d->replace);
            }
        } else if (strcmp(ts.tok, "defined") == 0) {
            // special case: handle the `defined(SYMBOL)` pseudo-macro
            handle_defined(&ts, &result, defs);
        } else {
            add_to_str(&result, ts.tok);
        }
    }
    return result.s;
}
