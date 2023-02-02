#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

static const char FLAGS[] = "t:";
static int time_limit = 15;

int make_int(char* str) {
    char *end;
    return strtol(str, &end, 10);
}

void resolve_args(int argc, char** argv) {
    while (1) {
        static struct option long_options[] = {
            {"time",    required_argument,    0,  't'}
        };
        int option_index = 0;

        int c = getopt_long(argc, argv, FLAGS, long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 't':
                if (!isdigit(*optarg)) {
                    fprintf(stderr, "argument passed to option -t must be an integer \n");
                    abort();
                }
                time_limit = make_int(optarg);
                break;
            default:
                abort();
        }
    }
}

int main(int argc, char** argv) {
    resolve_args(argc, argv);

    return 0;
}