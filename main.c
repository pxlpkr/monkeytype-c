#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

#include "lang.h"
#include "array.h"

#define K_BACKSPACE 127
#define K_ESCAPE 27

static int PADDING = 4;
int linewidth;

static int time_limit = 15;

void resolve_args(int argc, char** argv) {
    while (1) {
        static struct option long_options[] = {
            {"time",    required_argument,    0,  't'}
        };
        int option_index = 0;

        int c = getopt_long(argc, argv, "t:", long_options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 't':
                if (!isdigit(*optarg)) {
                    fprintf(stderr, "argument passed to option -t must be an integer \n");
                    exit(1);
                }
                char* end;
                time_limit = strtol(optarg, &end, 10);
                break;
            default:
                exit(1);
        }
    }
}

void make_line(char* string, int len) {
    *string = '\0';
    while (1) {
        const char *word = *(english_1k + rand() % english_1k_length);

        if ((int) strlen(string) + (int) strlen(word) + 1 >= len) return;

        strcat(string, word);
        strcat(string, " ");
    }
}

void init_ncurses(void) {
    initscr();
    start_color();
    use_default_colors();

    init_pair(1, COLOR_WHITE, -1);
    init_pair(2, -1, -1);
    init_pair(3, COLOR_RED, -1);

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, FALSE);

    linewidth = getmaxx(stdscr) - 2 * PADDING;
}

int assign_new_data(array_t* texta, array_t* colorsa) {
    char* string = malloc(linewidth + 1);
    make_line(string, linewidth + 1);
    array_append(texta, string);

    array_t* intarr = malloc(sizeof(array_t));
    array_make(intarr);
    array_append(colorsa, (void*) intarr);

    return 0;
}

int rerender(array_t* text, array_t* colors, int y) {
    int start_position = y - 1;
    if (start_position < 0) start_position = 0;
    move(1, 0);
    for (int i = start_position; i < start_position + 3; i++) {
        array_t* intarr = (array_t*) array_get(colors, i);
        char* c = text->at[i];

        if (start_position == 0) attron(COLOR_PAIR(1));
        else if (i == start_position) attron(COLOR_PAIR(2));
        else attron(COLOR_PAIR(1));

        printw("%s \n", c);

        if (start_position == 0) attroff(COLOR_PAIR(1));
        else if (i == start_position) attroff(COLOR_PAIR(2));
        else attroff(COLOR_PAIR(1));

        for (int j = 0; j < intarr->len; j++) {
            int* k = intarr->at[j];
            move(i - start_position + 1, *k);
            attron(COLOR_PAIR(3));
            printw("%c", c[*k]);
            attroff(COLOR_PAIR(3));
        }

        move(i - start_position + 2, 0);
    }

    return 0;
}

int end_test(void) {
    clear();
    move(0, 0);
    printw("wpm\n");
    printw("  114\n");
    printw("acc\n");
    printw("  92%%\n");
}

int main(int argc, char** argv) {
    resolve_args(argc, argv);
    init_ncurses();

    array_t text;
    array_make(&text);

    array_t colors;
    array_make(&colors);

    for (int i = 0; i < 3; i++) {
        assign_new_data(&text, &colors);
    }

    int y_pos = 0;
    int x_pos = 0;
    int timer = 0;

    rerender(&text, &colors, y_pos);

    while (1) {
        if (y_pos == 0) move(1, x_pos);
        else move(2, x_pos);
        int c = getch();

        if (c != -1) {
            char current = ((char*) array_get(&text, y_pos))[x_pos];

            if (c == K_ESCAPE) {
                endwin();

                exit(0);
            } else if (c == K_BACKSPACE) {
                attron(COLOR_PAIR(1));
                printw("%c", current);
                attroff(COLOR_PAIR(1));

                array_t* intarr = (array_t*) array_get(&colors, y_pos);

                x_pos--;

                if (x_pos >= 0) for (int i = 0; i < intarr->len; i++) {
                    if (*(int*) array_get(intarr, i) == x_pos) {
                        array_remove(intarr, i);
                    }
                }
            } else {
                int i;
                if (c == current) {
                    attron(COLOR_PAIR(2));
                    printw("%c", current);
                    attroff(COLOR_PAIR(2));
                } else {
                    attron(COLOR_PAIR(3));
                    printw("%c", current);
                    attroff(COLOR_PAIR(3));

                    array_t* intarr = (array_t*) array_get(&colors, y_pos);
                    int* error_index = malloc(sizeof(int));
                    *error_index = x_pos;
                    array_append(intarr, (void*) error_index);
                }
                x_pos++;
            }

            if (x_pos >= (int) strlen(array_get(&text, y_pos))) {
                y_pos++;
                x_pos = 0;
                if (y_pos == text.len - 1) {
                    assign_new_data(&text, &colors);
                }
                rerender(&text, &colors, y_pos);
            } else if (x_pos < 0) {
                if (y_pos > 0) {
                    y_pos--;
                    x_pos = (int) strlen(array_get(&text, y_pos)) - 1;
                    rerender(&text, &colors, y_pos);
                } else {
                    x_pos = 0;
                }
            }
        }

        if (time_limit * 1000 - timer == 2) {
            end_test();
            break;
        }

        if (timer % 1000 == 0) {
            move(0,0);
            printw("%i        ", time_limit - (timer / 1000));
        }

        usleep(1000);
        timer++;
    }

    while (1) {
        int c = getch();
        if (c != -1) {
            if (c == K_ESCAPE) {
                endwin();

                exit(0);
            }
        }

        usleep(1000);
    }

    return 0;
}