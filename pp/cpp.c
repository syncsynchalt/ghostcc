#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "defs.h"
#include "parse.h"

static char *infile = NULL;
static char *outfile = NULL;

void usage(int argc, char **argv)
{
    fprintf(stderr, "Usage: %s infile [[-o] outfile]\n", argv[0]);
    exit(1);
}


int main(const int argc, char **argv)
{
    int arg;

    while ((arg = getopt(argc, argv, "o:")) != -1) {
        if (arg == '?' || arg == ':') {
            usage(argc, argv);
        }
        if (arg == 'o') {
            outfile = optarg;
        }
    }
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

    parse(in, out, defs);
}
