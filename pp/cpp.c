#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "preprocessor.h"
#include "die.h"

static char *infile = NULL;
static char *outfile = NULL;

void usage(int argc, char **argv)
{
    die("Usage: %s infile [[-o] outfile] [-I include_path]", argv[0]);
}


int main(const int argc, char **argv)
{
    int arg;
    const char **include_paths = NULL;
    int num_include_paths = 0;

    include_paths = malloc(sizeof(*include_paths) * (num_include_paths + 6));
    include_paths[num_include_paths++] = strdup("/usr/include");
    FILE *p = popen("xcrun --show-sdk-path", "r");
    if (p) {
        char buf[512];
        fgets(buf, sizeof(buf), p);
        if (strlen(buf)) {
            const size_t len = strcspn(buf, "\r\n");
            snprintf(buf+len, sizeof(buf)-len, "/usr/include");
            include_paths[num_include_paths++] = strdup(buf);
        }
        pclose(p);
    }

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
        die("Can't open %s for reading: %s", infile, strerror(errno));
    }
    FILE *out = outfile ? fopen(outfile, "w") : stdout;
    if (!out) {
        die("Can't open %s for writing: %s", outfile, strerror(errno));
    }

    defines *defs = defines_init();

    parse_state state = {0};
    state.defs = defs;
    state.include_paths = include_paths;
    parse(infile, in, out, &state);
    fclose(out);
    fclose(in);

    int i;
    for (i = 0; i < num_include_paths; i++) {
        free((void *)include_paths[i]);
    }
    free(include_paths);
}
