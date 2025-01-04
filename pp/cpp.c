#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "preprocessor.h"

static char *infile = NULL;
static char *outfile = NULL;

void usage(int argc, char **argv)
{
    fprintf(stderr, "Usage: %s infile [[-o] outfile] [-I include_path]\n", argv[0]);
    exit(1);
}


int main(const int argc, char **argv)
{
    int arg;
    const char **include_paths = NULL;
    int num_include_paths = 0;

    include_paths = malloc(sizeof(*include_paths) * (num_include_paths + 6));
    include_paths[num_include_paths++] = strdup("/usr/include");

    while ((arg = getopt(argc, argv, "o:I:")) != -1) {
        if (arg == '?' || arg == ':') {
            usage(argc, argv);
        }
        if (arg == 'o') {
            outfile = optarg;
        }
        if (arg == 'I') {
            if (num_include_paths % 5 == 0) {
                include_paths = realloc(include_paths, sizeof(*include_paths) * 6);
                include_paths[num_include_paths++] = strdup(optarg);
            }
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

    FILE *in = fopen(infile, "r");
    if (!in) {
        fprintf(stderr, "Can't open %s for reading: %s\n", infile, strerror(errno));
        exit(1);
    }
    FILE *out = outfile ? fopen(outfile, "w") : stdout;
    if (!out) {
        fprintf(stderr, "Can't open %s for writing: %s\n", outfile, strerror(errno));
        exit(1);
    }

    const defines *defs = defines_init();

    parse(in, out, defs, include_paths);
    fclose(out);
    fclose(in);

    int i;
    for (i = 0; i < num_include_paths; i++) {
        free((void *)include_paths[i]);
    }
    free(include_paths);
}
