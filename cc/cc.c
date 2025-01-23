#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "parse.h"
#include "common.h"
#include "die.h"

static char *infile = NULL;
static char *outfile = NULL;
static char outfile_buf[256];

void usage(int argc, char **argv)
{
    die("Usage: %s infile [-o outfile] [-I include_path]", argv[0]);
}

char **builtin_include_paths(size_t *num_include_paths)
{
    char **include_paths = NULL;

    include_paths = malloc(sizeof(*include_paths) * (*num_include_paths + 6));
    include_paths[(*num_include_paths)++] = strdup("/usr/include");
    FILE *p = popen("xcrun --show-sdk-path", "r");
    if (p) {
        char buf[512];
        fgets(buf, sizeof(buf), p);
        if (strlen(buf)) {
            const size_t len = strcspn(buf, "\r\n");
            snprintf(buf+len, sizeof(buf)-len, "/usr/include");
            include_paths[(*num_include_paths)++] = strdup(buf);
        }
        pclose(p);
    }
    return include_paths;
}

static FILE *preprocess_file(const char *filename)
{
    char ppout[256] = "/tmp/ghostpp.tmp.XXXXXX";
    mkstemp(ppout);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ghostpp -o %s %s", ppout, filename);
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
    char **include_paths = builtin_include_paths(&num_include_paths);

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

    if (!outfile) {
        snprintf(outfile_buf, sizeof(outfile_buf), "%s", infile);
        char *p = strrchr(outfile_buf, '.');
        if (!p) {
            p = outfile_buf + strlen(outfile_buf);
        }
        snprintf(p, sizeof(outfile_buf) - (p - outfile_buf), ".o");
        outfile = outfile_buf;
    }
    FILE *out = fopen(outfile, "w");
    if (!out) {
        die("Can't open %s for writing: %s", outfile, strerror(errno));
    }
    output = out;

    FILE *in = preprocess_file(infile);
    process_file(infile, in, out);
    fclose(out);
    fclose(in);

    int i;
    for (i = 0; i < num_include_paths; i++) {
        free((void *)include_paths[i]);
    }
    free(include_paths);
}
