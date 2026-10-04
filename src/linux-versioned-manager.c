#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

// https://stackoverflow.com/a/30960382
#define _XOPEN_SOURCE_EXTENDED

#ifdef __MINGW32__
#include <ncurses/ncurses.h>
    #include <ncurses/panel.h>
#else
    #include <ncurses.h>
    #include <panel.h>
#endif

typedef struct sigaction sigaction_t;
typedef struct winsize winsize_t;
winsize_t winsz;

typedef struct {
    const char* str;
    const unsigned int len;
} nstr;

typedef struct {
    union {
        int row;
        int y;
    };
    union {
        int col;
        int x;
    };
} pos_t;

WINDOW *mainScreen = NULL;
PANEL *mainPanel = NULL;

char displayText[8] = ""; 

nstr nstrCreate(const char* str);
int customBorder(WINDOW* win, nstr* title, pos_t start, pos_t end);

// https://stackoverflow.com/a/13707598
static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                endwin();
                refresh();
                clear();
                box(mainScreen, LINES, COLS);
                sprintf(displayText, "%3d %3d", LINES, COLS);
                mvwaddstr(mainScreen, 2, 2, displayText);
                refresh();
                wrefresh(mainScreen);
            }
            break;
        default:
    }
}

int main(int argc, char* argv[]) {
    sigaction_t sa = { .sa_handler=handler };
    sigaction(SIGWINCH, &sa, NULL);
    
    initscr(); cbreak(); noecho();
    keypad(stdscr, TRUE);
    sprintf(displayText, "%3d %3d", LINES, COLS);

    nstr mainTitle = nstrCreate("kernel varients");

    mainScreen = newwin(LINES, COLS, 0, 0);
    pos_t mainScreenStartPos = {0, 0};
    pos_t mainScreenEndPos   = {LINES, COLS};
    customBorder(mainScreen, &mainTitle, mainScreenStartPos, mainScreenEndPos);
    mvwaddstr(mainScreen, 2, 2, displayText);
    refresh();
    wrefresh(mainScreen);
    mainPanel = new_panel(mainScreen);

    while(getch() != 'q');
    endwin();
    return 0;
}

nstr nstrCreate(const char* str) {
    unsigned int len = strlen(str);
    nstr tmp = {.str=str, .len=len};
    return tmp;
}

int customBorder(WINDOW* win, nstr* title, pos_t start, pos_t end) {
    // https://stackoverflow.com/a/69492307
    // https://stackoverflow.com/a/35712716
    cchar_t lhChar = {};
    setcchar(&lhChar, L"\u2502", NULL, NULL, NULL);
    mvwprintw(win, 0, 0, "\u250c%*s\u2510", end.col-1, "\u2500");
    mvwvline_set(win, 1, 0, &lhChar, start.row-1);
    mvwvline_set(win, 1, end.col, &lhChar, end.row-1);
    mvwprintw(win, end.row, 0, "\u2514%*s\u2518", end.col-1, "─");

    return 0;
}
