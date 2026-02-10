#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CMD_LEN 256
#define MAX_OUT 8192

int execute_command_backend(char *command, char *output);

/* ---------- FILE PANEL ---------- */
void show_files(WINDOW *win) {
    DIR *d;
    struct dirent *dir;
    int y = 1;

    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " FILES ");

    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) && y < getmaxy(win) - 1) {
            mvwprintw(win, y++, 2, "%s", dir->d_name);
        }
        closedir(d);
    }
    wrefresh(win);
}

/* ---------- OUTPUT PANEL ---------- */
void run_command(char *cmd, WINDOW *out) {
    char output[MAX_OUT];
    memset(output, 0, sizeof(output));

    werase(out);
    box(out, 0, 0);
    mvwprintw(out, 0, 2, " OUTPUT ");

    execute_command_backend(cmd, output);

    if (strlen(output) == 0) {
        mvwprintw(out, 1, 2, "Command executed successfully");
    } else {
        mvwprintw(out, 1, 2, "%s", output);
    }

    wrefresh(out);
}

int main() {
    char command[CMD_LEN];

    /* ---------- NCURSES INIT (WHITE SCREEN) ---------- */
    initscr();
    start_color();
    use_default_colors();

    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    wbkgd(stdscr, COLOR_PAIR(1));
    clear();
    refresh();

    /* ---------- INPUT BEHAVIOR (FIXED) ---------- */
    cbreak();                 // immediate input
    echo();                   // SHOW what user types
    keypad(stdscr, TRUE);     // handle arrow keys
    curs_set(1);

    /* Ignore mouse completely */
    mousemask(0, NULL);

    int height = LINES - 3;
    int left_w = COLS / 3;

    WINDOW *files  = newwin(height, left_w, 0, 0);
    WINDOW *output = newwin(height, COLS - left_w, 0, left_w);
    WINDOW *input  = newwin(3, COLS, height, 0);

    wbkgd(files,  COLOR_PAIR(1));
    wbkgd(output, COLOR_PAIR(1));
    wbkgd(input,  COLOR_PAIR(1));

    show_files(files);

    while (1) {
        werase(input);
        box(input, 0, 0);
        mvwprintw(input, 1, 2, "Command: ");
        wmove(input, 1, 11);
        wrefresh(input);

        wgetnstr(input, command, CMD_LEN - 1);

        if (strcmp(command, "exit") == 0)
            break;

        run_command(command, output);
        show_files(files);
    }

    endwin();
    return 0;
}
