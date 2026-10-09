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
#else
    #include <curses.h>
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
    WINDOW* win;
    pos_t startPos;
    pos_t endPos;
    pos_t prevEndPos;
} win_t;

char endPosText[8] = ""; 
char prevEndPosText[8] = ""; 
static nstr* variantSelectorTitlePtr = 0;
static nstr* versionSelectorTitlePtr = 0;
win_t mainScreen = {};
win_t variantSelectorWin  = {};
win_t variantSelectorInfo = {};
win_t versionSelectorWin  = {};
win_t versionSelectorInfo = {};

nstr nstrCreate(char* str);
int customBorder(WINDOW* win, nstr* title, win_t* winInfo);
int cleanLingeringBorder(WINDOW* win, win_t* winInfo);

// https://stackoverflow.com/a/13707598
static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                endwin();
                refresh();
                clear();
                mainScreen.endPos = (pos_t){LINES, COLS};
                variantSelectorWin.endPos  = (pos_t){LINES, 24};
                variantSelectorInfo.endPos = (pos_t){LINES-4, 20};
                versionSelectorWin.endPos  = (pos_t){LINES, COLS-24};
                versionSelectorInfo.endPos = (pos_t){LINES-4, COLS-28};
                delwin(variantSelectorInfo.win);
                delwin(versionSelectorInfo.win);
                delwin(versionSelectorWin.win);
                variantSelectorInfo.win = subwin(variantSelectorWin.win, LINES-4, 20, 2, 2);
                versionSelectorWin.win  = subwin(mainScreen.win, LINES, COLS-24, 0, 24);
                versionSelectorInfo.win = subwin(versionSelectorWin.win, LINES-4, COLS-28, 2, 26);
                wsyncup(variantSelectorInfo.win);
                wsyncup(versionSelectorInfo.win);
                sprintf(endPosText, "%3d %3d", LINES, COLS);
                customBorder(variantSelectorWin.win, variantSelectorTitlePtr, &variantSelectorWin);
                customBorder(versionSelectorWin.win, versionSelectorTitlePtr, &versionSelectorWin);
                mvwaddstr(variantSelectorInfo.win, 0, 0, endPosText);
                refresh();
                wrefresh(variantSelectorWin.win);
                wrefresh(versionSelectorWin.win);
            }
            break;
        default:
    }
}

int main(int argc, char* argv[]) {
    sigaction_t sa = { .sa_handler=handler };
    sigaction(SIGWINCH, &sa, NULL);
    
    nstr variantSelectorTitle = nstrCreate("kernel varients"); 
    variantSelectorTitlePtr = &variantSelectorTitle;
    nstr versionSelectorTitle = nstrCreate("kernel versions"); 
    versionSelectorTitlePtr = &versionSelectorTitle;

    setlocale(LC_ALL, "");
    initscr(); cbreak(); noecho();
    keypad(stdscr, TRUE);
    sprintf(endPosText, "%3d %3d", LINES, COLS);

    mainScreen.win = newwin(LINES, COLS, 0, 0);
    mainScreen.startPos = (pos_t){0, 0};
    mainScreen.endPos   = (pos_t){LINES, COLS};
    mainScreen.prevEndPos = mainScreen.endPos;

    variantSelectorWin.win = subwin(mainScreen.win, LINES, 24, 0, 0);
    variantSelectorWin.startPos   = (pos_t){0, 0};
    variantSelectorWin.endPos     = (pos_t){LINES, 24};
    variantSelectorWin.prevEndPos = variantSelectorWin.endPos;

    variantSelectorInfo.win = subwin(variantSelectorWin.win, LINES-4, 20, 2, 2);
    variantSelectorInfo.startPos   = (pos_t){0, 0};
    variantSelectorInfo.endPos     = (pos_t){LINES-4, 20};
    variantSelectorInfo.prevEndPos = variantSelectorInfo.endPos;

    versionSelectorWin.win = subwin(mainScreen.win, LINES, COLS-24, 0, 24);
    versionSelectorWin.startPos   = (pos_t){0, 0};
    versionSelectorWin.endPos     = (pos_t){LINES, COLS-24};
    versionSelectorWin.prevEndPos = versionSelectorWin.endPos;

    versionSelectorInfo.win = subwin(versionSelectorWin.win, LINES-4, COLS-28, 2, 26);
    versionSelectorInfo.startPos   = (pos_t){0, 0};
    versionSelectorInfo.endPos     = (pos_t){LINES-4, COLS-28};
    versionSelectorInfo.prevEndPos = versionSelectorInfo.endPos;
    
    customBorder(variantSelectorWin.win, variantSelectorTitlePtr, &variantSelectorWin);
    customBorder(versionSelectorWin.win, versionSelectorTitlePtr, &versionSelectorWin);
    mvwaddnstr(variantSelectorInfo.win, 0, 0, endPosText, 8);
    refresh();
    wsyncup(variantSelectorInfo.win);
    wsyncup(versionSelectorInfo.win);
    wrefresh(mainScreen.win);

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

int customBorder(WINDOW* win, nstr* title, win_t* winInfo) {
    // Historical record
    // UNUSED: https://stackoverflow.com/a/69492307
    // UNUSED: https://stackoverflow.com/a/35712716
    cleanLingeringBorder(win, winInfo);
    winInfo->prevEndPos = winInfo->endPos;

    wmove(win, 0, 0);
    waddch(win, ACS_ULCORNER);
    if(title->len+8 <= winInfo->endPos.col) {
        waddch(win, ACS_HLINE);
        waddch(win, ACS_HLINE);
        waddch(win, ' ');
        waddnstr(win, title->str, title->len);
        waddch(win, ' ');
        whline(win, ACS_HLINE, winInfo->endPos.col-(6+title->len));
    }
    else whline(win, ACS_HLINE, winInfo->endPos.col-2);
    wmove(win, 0, winInfo->endPos.col-1);
    waddch(win, ACS_URCORNER);
    
    wmove(win, 1, 0);
    wvline(win, ACS_VLINE, winInfo->endPos.row-1);
    wmove(win, 1, winInfo->endPos.col-1);
    wvline(win, ACS_VLINE, winInfo->endPos.row-1);

    wmove(win, winInfo->endPos.row-1, 0);
    waddch(win, ACS_LLCORNER);
    whline(win, ACS_HLINE, winInfo->endPos.col-2);
    wmove(win, winInfo->endPos.row-1, winInfo->endPos.col-1);
    waddch(win, ACS_LRCORNER);

    return 0;
}

int cleanLingeringBorder(WINDOW* win, win_t* winInfo) {
    if(winInfo->prevEndPos.row < LINES) {
        wmove(win, winInfo->prevEndPos.row-1, 1);
        for(int i = 1; i < winInfo->prevEndPos.col; i++) waddch(win, ' ');
    }
    if(winInfo->prevEndPos.col < COLS) {
        for(int i = 1; i < winInfo->prevEndPos.row; i++) {
            wmove(win, i, winInfo->prevEndPos.col-1);
            waddch(win, ' ');
        }
    }

    return 0;
}