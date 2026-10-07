#include <locale.h>
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

typedef struct {
    pos_t startPos;
    pos_t endPos;
    pos_t prevEndPos;
} winInfo_t;

WINDOW *mainScreen = NULL;

char endPosText[8] = ""; 
char prevEndPosText[8] = ""; 
static nstr* mainTitlePtr = 0;
winInfo_t mainScreenInfo = {};

nstr nstrCreate(char* str);
int customBorder(WINDOW* win, nstr* title, winInfo_t* winInfo);
int cleanLingeringBorder(WINDOW* win, pos_t prevStartPos, pos_t prevEndPos);

// https://stackoverflow.com/a/13707598
static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                endwin();
                refresh();
                clear();
                mainScreenInfo.endPos = (pos_t){LINES, COLS};
                sprintf(endPosText, "%3d %3d", LINES, COLS);
                sprintf(prevEndPosText, "%3d %3d", mainScreenInfo.prevEndPos.row, mainScreenInfo.prevEndPos.col);
                customBorder(mainScreen, mainTitlePtr, &mainScreenInfo);
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
    
    nstr mainTitle = nstrCreate("kernel varients"); 
    mainTitlePtr = &mainTitle;

    setlocale(LC_ALL, "");
    initscr(); cbreak(); noecho();
    keypad(stdscr, TRUE);
    sprintf(endPosText, "%3d %3d", LINES, COLS);
    sprintf(prevEndPosText, "%3d %3d", mainScreenInfo.prevEndPos.row, mainScreenInfo.prevEndPos.col);

    mainScreen = newwin(LINES, COLS, 0, 0);
    mainScreenInfo.startPos = (pos_t){0, 0};
    mainScreenInfo.endPos   = (pos_t){LINES, COLS};
    mainScreenInfo.prevEndPos = mainScreenInfo.endPos;
    customBorder(mainScreen, mainTitlePtr, &mainScreenInfo);
    mvwaddnstr(mainScreen, 2, 2, endPosText, 8);
    refresh();
    wrefresh(mainScreen);

    while(getch() != 'q');
    endwin();
    return 0;
}

// https://stackoverflow.com/a/65848356
// https://stackoverflow.com/a/73945800
nstr nstrCreate(char* str) {
    unsigned int len = strlen(str);
    return (nstr){str, len};
}

int customBorder(WINDOW* win, nstr* title, winInfo_t* winInfo) {
    // Historical record
    // UNUSED: https://stackoverflow.com/a/69492307
    // UNUSED: https://stackoverflow.com/a/35712716
    cleanLingeringBorder(win, winInfo->startPos, winInfo->prevEndPos);
    winInfo->prevEndPos = winInfo->endPos;

    wmove(win, 0, 0);
    waddch(win, ACS_ULCORNER);
    waddch(win, ACS_HLINE);
    waddch(win, ACS_HLINE);
    waddch(win, ' ');
    waddnstr(win, title->str, title->len);
    waddch(win, ' ');
    for(int i = 6+title->len; i < winInfo->endPos.col; i++) waddch(win, ACS_HLINE);
    waddch(win, ACS_URCORNER);
    
    for(int i = 1; i < winInfo->endPos.row; i++) {
        wmove(win, i, 0);
        waddch(win, ACS_VLINE);
        wmove(win, i, winInfo->endPos.col-1);
        waddch(win, ACS_VLINE);
    }
    wmove(win, winInfo->endPos.row-1, 0);
    waddch(win, ACS_LLCORNER);
    for(int i = 2; i < winInfo->endPos.col; i++) waddch(win, ACS_HLINE);
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