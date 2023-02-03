#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <ncurses.h>
#include <string.h>

#include "ansi.h"
#include "lang.h"

static const char FLAGS[] = "t:";

static int padding = 4;
static int time_limit = 15;

/* Convert a string into an integer value */
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

        if (c == -1) break;

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

void make_line(char* string, int len) {
    while (1) {
        const char *word = *(english_1k + rand() % english_1k_length);

        if ((int) strlen(string) + (int) strlen(word) + 1 >= len) return;

        strcat(string, word);
        strcat(string, " ");
    }
}

void init_ncurses() {
    initscr();
    start_color();
    use_default_colors();

    init_pair(1, COLOR_BLACK, -1);
    init_pair(2, COLOR_WHITE, -1);
    init_pair(3, COLOR_RED, -1);

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, FALSE);
}

int main(int argc, char** argv) {
    resolve_args(argc, argv);
    
    init_ncurses();

    const int LINEWIDTH = getmaxx(stdscr) - 2 * padding;
    char prompts[3][LINEWIDTH];
    strcpy(prompts[0], "");

    for (int i = 0; i < 3; i++) {
        char string[LINEWIDTH + 1];
        memset(string, 0, sizeof(char) * (LINEWIDTH + 1));
        make_line(string, LINEWIDTH + 1);
        strcpy(prompts[i], string);
    }

    attron(COLOR_PAIR(1));
    for (int i = 0; i < 3; i++) {
        printw("%-4s%s\n", "", prompts[i]);
    }
    attroff(COLOR_PAIR(1));
    move(0, padding);

    int x_pos = 0;
    int y_pos = 0;

    while (1) {
        move(y_pos, x_pos + padding);
        int c = getch();

        if (c != -1) {
            char cur_char = prompts[y_pos][x_pos];
            if (c == 127) {
                attron(COLOR_PAIR(1));
                printw("%c", cur_char);
                attroff(COLOR_PAIR(1));
                x_pos--;
            } else {
                if (c == cur_char) {
                    attron(COLOR_PAIR(2));
                    printw("%c", cur_char);
                    attroff(COLOR_PAIR(2));
                } else {
                    attron(COLOR_PAIR(3));
                    printw("%c", cur_char);
                    attroff(COLOR_PAIR(3));
                }
                x_pos++;
            }

            if (x_pos >= (int) strlen(prompts[y_pos])) {
                y_pos++;
                x_pos = 0;
            } else if (x_pos < 0) {
                if (y_pos > 0) {
                    y_pos--;
                    x_pos = (int) strlen(prompts[y_pos]) - 1;
                } else {
                    x_pos = 0;
                }
            }
        }
    }

    return 0;
}