#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse.h"
#include "common.h"
#include "die.h"

static char *infile = NULL;
static char *outfile = "a.out";

void usage(int argc, char **argv)
{
    die("Usage: %s infile [-o outfile] [-I include_path]", argv[0]);
}

static FILE *preprocess_file(const char *filename, char **include_paths)
{
    char ppout[256] = "/tmp/ghostpp.tmp.XXXXXX";
    mkstemp(ppout);
    char cmd[1024];

    snprintf(cmd, sizeof(cmd), "ghostpp -o %s", ppout);
    size_t i, l;
    for (i = 0; include_paths[i]; i++) {
        l = strlen(cmd);
        snprintf(cmd + l, sizeof(cmd) - l, " -I \"%s\"", include_paths[i]);
    }
    l = strlen(cmd);
    snprintf(cmd + l, sizeof(cmd) - l, " %s", filename);

    if (system(cmd) != 0) {
        die("Error running '%s', exiting", cmd);
    }
    FILE *f = fopen(ppout, "r");
    if (!f) {
        die("Can't open %s: %s", ppout, strerror(errno));
    }
    unlink(ppout);
    return f;
}

int main(const int argc, char **argv)
{
    int arg;
    size_t num_include_paths = 0;
    char **include_paths = calloc(1, sizeof(*include_paths) * 1);

    while ((arg = getopt(argc, argv, "o:I:")) != -1) {
        if (arg == '?' || arg == ':') {
            usage(argc, argv);
        }
        if (arg == 'o') {
            outfile = optarg;
        }
        if (arg == 'I') {
            if (num_include_paths % 5 == 0) {
                include_paths = realloc(include_paths, sizeof(*include_paths) * (num_include_paths + 6));
            }
            include_paths[num_include_paths++] = strdup(optarg);
        }
    }
    include_paths[num_include_paths] = NULL;
    if (optind >= argc) {
        usage(argc, argv);
    }

    infile = argv[optind++];
    if (!outfile && optind < argc) {
        outfile = argv[optind++];
    }
    if (optind < argc) {
        usage(argc, argv);
    }

    FILE *out = fopen(outfile, "w");
    if (!out) {
        die("Can't open %s for writing: %s", outfile, strerror(errno));
    }
    output = out;

    FILE *in = preprocess_file(infile, include_paths);
    process_file(infile, in, out);
    fclose(out);
    fclose(in);

    int i;
    for (i = 0; i < num_include_paths; i++) {
        free((void *)include_paths[i]);
    }
    free(include_paths);
}
