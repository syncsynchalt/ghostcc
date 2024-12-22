#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

static void process_directive(const char *line, size_t line_len, FILE *out, const defines *defs);
static const char *process_token(const char *tok, const char *line, size_t line_len, FILE *out, const defines *defs);

void parse(FILE *in, FILE *out, const defines *defs)
{
    char *line;
    size_t linecap;
    size_t len;
    const char *tok;

    while ((len = getline(&line, &linecap, in)) > 0) {
        if (line[0] == '#') {
            process_directive(line, len, out, defs);
        } else {
            tok = line;
            while (tok) {
                tok = process_token(tok, line, len, out, defs);
            }
        }
    }
    if (ferror(in)) {
        perror("reading input file");
        exit(1);
    }
}

static void process_directive(const char *line, const size_t line_len, FILE *out, const defines *defs)
{
    fwrite(line, 1, line_len, out);
}

static const char *process_token(const char *tok, const char *line, const size_t line_len, FILE *out,
                                 const defines *defs)
{
    int len;

    len = strspn(tok, WHITESPACE);
    if (len) {
        // whitespace token
        fwrite(tok, len, 1, out);
        return tok + len;
    } else {
        // normal token
        // TODO handle define(), define, etc.
        len = strcspn(tok, WHITESPACE);
        if (len) {
            fwrite(tok, len, 1, out);
            return tok + len;
        } else {
            return NULL;
        }
    }
}
