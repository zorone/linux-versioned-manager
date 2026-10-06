#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
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

char endPosText[8] = ""; 
char prevEndPosText[8] = ""; 
nstr mainTitle = {};
pos_t mainScreenStartPos   = {};
pos_t mainScreenEndPos     = {};
pos_t mainScreenPrevEndPos = {};

nstr nstrCreate(const char* str);
int customBorder(WINDOW* win, nstr* title, pos_t start, pos_t end);
int cleanLingeringBorder(WINDOW* win, pos_t prevStartPos, pos_t prevEndPos);

// https://stackoverflow.com/a/13707598
static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                endwin();
                refresh();
                clear();
                mainScreenEndPos = (pos_t){LINES, COLS};
                sprintf(endPosText, "%3d %3d", LINES, COLS);
                sprintf(prevEndPosText, "%3d %3d", mainScreenPrevEndPos.row, mainScreenPrevEndPos.col);
                customBorder(mainScreen, &mainTitle, mainScreenStartPos, mainScreenEndPos);
                mvwaddstr(mainScreen, 2, 2, endPosText);
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
    sprintf(endPosText, "%3d %3d", LINES, COLS);
    sprintf(prevEndPosText, "%3d %3d", mainScreenPrevEndPos.row, mainScreenPrevEndPos.col);

    nstr mainTitle = nstrCreate("kernel varients");

    mainScreen = newwin(LINES, COLS, 0, 0);
    mainScreenStartPos.row = 0;
    mainScreenStartPos.col = 0;
    mainScreenEndPos = (pos_t){LINES, COLS};
    mainScreenPrevEndPos = mainScreenEndPos;
    customBorder(mainScreen, &mainTitle, mainScreenStartPos, mainScreenEndPos);
    mvwaddnstr(mainScreen, 2, 2, endPosText, 8);
    mvwaddnstr(mainScreen, 3, 2, prevEndPosText, 8);
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
    // Historical record
    // UNUSED: https://stackoverflow.com/a/69492307
    // UNUSED: https://stackoverflow.com/a/35712716
    mvwaddstr(mainScreen, 2, 2, endPosText);
    mvwaddstr(mainScreen, 3, 2, prevEndPosText);
    refresh();
    wrefresh(mainScreen);
    sleep(1);
    cleanLingeringBorder(mainScreen, mainScreenStartPos, mainScreenPrevEndPos);
    mvwaddstr(mainScreen, 2, 2, endPosText);
    mvwaddstr(mainScreen, 3, 2, prevEndPosText);
    refresh();
    wrefresh(mainScreen);
    sleep(1);
    mainScreenPrevEndPos = mainScreenEndPos;
    sprintf(prevEndPosText, "%3d %3d", mainScreenPrevEndPos.row, mainScreenPrevEndPos.col);
    mvwaddstr(mainScreen, 3, 2, prevEndPosText);

    wmove(win, 0, 0);
    waddch(win, ACS_ULCORNER);
    for(int i = 2; i < end.col; i++) waddch(win, ACS_HLINE);
    waddch(win, ACS_URCORNER);
    
    for(int i = 1; i < end.row; i++) {
        wmove(win, i, 0);
        waddch(win, ACS_VLINE);
        wmove(win, i, end.col-1);
        waddch(win, ACS_VLINE);
    }
    wmove(win, end.row-1, 0);
    waddch(win, ACS_LLCORNER);
    for(int i = 2; i < end.col; i++) waddch(win, ACS_HLINE);
    waddch(win, ACS_LRCORNER);

    return 0;
}

int cleanLingeringBorder(WINDOW* win, pos_t prevStartPos, pos_t prevEndPos) {
    if(prevEndPos.row < LINES) {
        wmove(win, prevEndPos.row-1, 1);
        for(int i = 1; i < prevEndPos.col; i++) waddch(win, ' ');
    }
    if(prevEndPos.col < COLS) {
        for(int i = 1; i < prevEndPos.row; i++) {
            wmove(win, i, prevEndPos.col-1);
            waddch(win, ' ');
        }
    }

    return 0;
}