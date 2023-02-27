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

typedef struct {
    array_t text;
    array_t errors;
    
    int accuracy_correct;
    int accuracy_incorrect;

    int y_pos;
    int x_pos;

    int timer;

    int mode_time;
    int mode_words;
    int mode_quote;
    int mode_zen;
} test_context_t;

void multiple_mode_select_error(void) {
    fprintf(stderr, "cannot select mode multiple times; modes: (-t, -w, -z, -q) \n");
    exit(1);
}

void resolve_args(int argc, char** argv, test_context_t* ctx) {
    int mode_selected = 0;
    
    while (1) {
        static struct option long_options[] = {
            {"time",    optional_argument,  0,  't'},
            {"words",   optional_argument,  0,  'w'},
            {"quote",   optional_argument,  0,  'q'},
            {"zen",     no_argument,        0,  'z'}
        };
        int option_index = 0;

        int c = getopt_long(argc, argv, ":t:w:q:z", long_options, &option_index);

        if (c == -1) break;

        char* end;
        switch (c) {
            case ':':
                switch (optopt) {
                    case 't':
                        ctx->mode_time = 30;
                        break;
                    case 'w':
                        ctx->mode_words = 50;
                        break;
                    case 'q':
                        ctx->mode_quote = 3;
                        break;
                }
            case 't':
            case 'w':
                if (!isdigit(*optarg)) {
                    fprintf(stderr, "argument passed to option -%c must be an integer \n", c);
                    exit(1);
                }
                if (mode_selected) multiple_mode_select_error();
                long value = strtol(optarg, &end, 10);
                mode_selected = 1;
                if      (c == 't')  ctx->mode_time  = (int) value;
                else if (c == 'w')  ctx->mode_words = (int) value;
                break;
            case 'q':
                if (mode_selected) multiple_mode_select_error();
                if      (strcmp(optarg, "all"))     ctx->mode_quote = 1;
                else if (strcmp(optarg, "short"))   ctx->mode_quote = 2;
                else if (strcmp(optarg, "medium"))  ctx->mode_quote = 3;
                else if (strcmp(optarg, "long"))    ctx->mode_quote = 4;
                else if (strcmp(optarg, "thicc"))   ctx->mode_quote = 5;
                else {
                    fprintf(stderr, "argument passed to option -%c must be one of (all, short, medium, long, thicc) \n", c);
                    exit(1);
                }
                mode_selected = 1;
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

int assign_new_data(test_context_t* ctx) {
    char* string = malloc(linewidth + 1);
    make_line(string, linewidth + 1);
    array_append(&ctx->text, string);

    array_t* intarr = malloc(sizeof(array_t));
    array_make(intarr);
    array_append(&ctx->errors, (void*) intarr);

    return 0;
}

int rerender(test_context_t* ctx) {
    int start_position = ctx->y_pos - 1;;
    if (ctx->y_pos == 0) start_position = 0;

    move(1, 0);
    for (int i = start_position; i < start_position + 3; i++) {
        array_t* intarr = (array_t*) ctx->errors.at[i];
        char* c = ctx->text.at[i];

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

float get_acc(test_context_t* ctx) {
    int accuracy_total = ctx->accuracy_correct + ctx->accuracy_incorrect;
    if (accuracy_total == 0) return (float) 0;
    else                     return (float) ctx->accuracy_correct / accuracy_total;
}

float get_wpm(test_context_t* ctx) {
    int correct_word_chars = 0;

    for (int i = 0; i < ctx->y_pos + 1; i++) {
        char* string = ctx->text.at[i];
        
        int rolling_chars = 0;
        bool failed_word = false;

        int iter_len = (int) strlen(string);
        if (i == ctx->y_pos) iter_len = ctx->x_pos + 1;

        for (int j = 0; j < iter_len; j++) {
            char c = string[j];
            rolling_chars++;

            array_t* intarr = ctx->errors.at[i];
            for (int b = 0; b < intarr->len; b++) {
                if (*(int*) intarr->at[b] == j) {
                    failed_word = true;
                    break;
                }
            }

            if (c != ' ') continue;

            if (!failed_word) correct_word_chars += rolling_chars;
            rolling_chars = 0;
        }
    }

    float time_elapsed = (float) (ctx->timer) / 1000;

    return ((correct_word_chars) * (60 / time_elapsed)) / 5;
}

int end_test(test_context_t* ctx) {
    printf("wpm | %i\n", (int) get_wpm(ctx));
    printf("acc | %i%%\n", (int) (100*get_acc(ctx)));

    return 0;
}

void kp_correct(test_context_t* ctx, int c) {
    attron(COLOR_PAIR(2));
    printw("%c", c);
    attroff(COLOR_PAIR(2));

    ctx->accuracy_correct++;
    ctx->x_pos++;
}

void kp_incorrect(test_context_t* ctx, int c) {
    attron(COLOR_PAIR(3));
    printw("%c", c);
    attroff(COLOR_PAIR(3));

    array_t* intarr = (array_t*) ctx->errors.at[ctx->y_pos];
    int* error_index = malloc(sizeof(int));
    *error_index = ctx->x_pos;
    array_append(intarr, (void*) error_index);

    ctx->accuracy_incorrect++;
    ctx->x_pos++;
}

void kp_space(test_context_t* ctx) {
    ctx->x_pos++;
}

void kp_backspace(test_context_t* ctx, int c) {
    attron(COLOR_PAIR(1));
    printw("%c", c);
    attroff(COLOR_PAIR(1));

    array_t* intarr = (array_t*) ctx->errors.at[ctx->y_pos];

    ctx->x_pos--;

    if (ctx->x_pos >= 0) for (int i = 0; i < intarr->len; i++) {
        if (*(int*) array_get(intarr, i) == ctx->x_pos) {
            array_remove(intarr, i);
        }
    }
}

void kp_escape(void) {
    endwin();
    exit(0);
}

void scroll_up(test_context_t* ctx) {
    if (ctx->y_pos > 0) {
        ctx->y_pos--;
        ctx->x_pos = (int) strlen(ctx->text.at[ctx->y_pos]) - 1;
        rerender(ctx);
    } else {
        ctx->x_pos = 0;
    }
}

void scroll_down(test_context_t* ctx) {
    ctx->y_pos++;
    ctx->x_pos = 0;
    if (ctx->y_pos == ctx->text.len - 1) {
        assign_new_data(ctx);
    }
    rerender(ctx);
}

void update_timer(test_context_t* ctx) {
    move(0,0);
    printw("%i        ", ctx->mode_time - (ctx->timer / 1000));
}

void initialize_context(test_context_t* ctx) {
    array_make(&ctx->text);
    array_make(&ctx->errors);
    ctx->y_pos = 0;
    ctx->x_pos = 0;
    ctx->accuracy_correct = 0;
    ctx->accuracy_incorrect = 0;
    ctx->timer = 0;

    ctx->mode_time = 0;
    ctx->mode_words = 0;
    ctx->mode_zen = 0;
    ctx->mode_quote = 0;
}

void resolve_context_mode(test_context_t* ctx) {
    if (ctx->mode_quote == 0 && ctx->mode_time == 0 && ctx->mode_words == 0 && ctx->mode_zen == 0) {
        ctx->mode_time = 30;
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));

    /** Setting up the context */
    test_context_t ctx;
    initialize_context(&ctx);
    resolve_args(argc, argv, &ctx);
    resolve_context_mode(&ctx);

    /** Setting up the window */
    init_ncurses();
    for (int i = 0; i < 3; i++) assign_new_data(&ctx);
    rerender(&ctx);

    /** Main loop */
    while (1) {
        if (ctx.y_pos == 0)     move(1, ctx.x_pos);
        else                    move(2, ctx.x_pos);
        int c = getch();

        /** User pressed a key */
        if (c != -1) {
            char current = ((char*) ctx.text.at[ctx.y_pos])[ctx.x_pos];

            if      (c == K_ESCAPE)                                 kp_escape();
            else if (c == K_BACKSPACE)                              kp_backspace(&ctx, current);
            else if (current == ' ')                                kp_space(&ctx);
            else if (c == current)                                  kp_correct(&ctx, current);
            else                                                    kp_incorrect(&ctx, current);

            if (ctx.x_pos >= (int) strlen(ctx.text.at[ctx.y_pos]))  scroll_down(&ctx);
            else if (ctx.x_pos < 0)                                 scroll_up(&ctx);
        }

        if (ctx.mode_time * 1000 - ctx.timer == 2)                  break;
        if (ctx.timer % 1000 == 0)                                  update_timer(&ctx);

        usleep(1000); //Relieve the CPU
        ctx.timer++;
    }

    endwin();
    end_test(&ctx);

    return 0;
}