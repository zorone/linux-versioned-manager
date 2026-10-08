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
    pos_t startPos;
    pos_t endPos;
    pos_t prevEndPos;
} winInfo_t;

WINDOW *mainScreen = NULL;
WINDOW *variantSelectorWin = NULL;
WINDOW *versionSelectorWin = NULL;

char endPosText[8] = ""; 
char prevEndPosText[8] = ""; 
static nstr* variantSelectorTitlePtr = 0;
static nstr* versionSelectorTitlePtr = 0;
winInfo_t mainScreenInfo = {};
winInfo_t variantSelectorWinInfo = {};
winInfo_t versionSelectorWinInfo = {};

nstr nstrCreate(char* str);
int customBorder(WINDOW* win, nstr* title, winInfo_t* winInfo);
int cleanLingeringBorder(WINDOW* win, winInfo_t* winInfo);

// https://stackoverflow.com/a/13707598
static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                endwin();
                refresh();
                clear();
                mainScreenInfo.endPos = (pos_t){LINES, COLS};
                variantSelectorWinInfo.endPos = (pos_t){LINES, 24};
                versionSelectorWinInfo.endPos = (pos_t){LINES, COLS-24};
                sprintf(endPosText, "%3d %3d", LINES, COLS);
                sprintf(prevEndPosText, "%3d %3d", mainScreenInfo.prevEndPos.row, mainScreenInfo.prevEndPos.col);
                customBorder(variantSelectorWin, variantSelectorTitlePtr, &variantSelectorWinInfo);
                customBorder(versionSelectorWin, versionSelectorTitlePtr, &versionSelectorWinInfo);
                mvwaddstr(mainScreen, 2, 2, endPosText);
                refresh();
                wrefresh(variantSelectorWin);
                wrefresh(versionSelectorWin);
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
    sprintf(prevEndPosText, "%3d %3d", mainScreenInfo.prevEndPos.row, mainScreenInfo.prevEndPos.col);

    mainScreen = newwin(LINES, COLS, 0, 0);
    mainScreenInfo.startPos = (pos_t){0, 0};
    mainScreenInfo.endPos   = (pos_t){LINES, COLS};
    mainScreenInfo.prevEndPos = mainScreenInfo.endPos;

    variantSelectorWin = subwin(mainScreen, LINES, 24, 0, 0);
    variantSelectorWinInfo.startPos   = (pos_t){0, 0};
    variantSelectorWinInfo.endPos     = (pos_t){LINES, 24};
    variantSelectorWinInfo.prevEndPos = variantSelectorWinInfo.endPos;

    versionSelectorWin = subwin(mainScreen, LINES, COLS-24, 0, 24);
    versionSelectorWinInfo.startPos   = (pos_t){0, 0};
    versionSelectorWinInfo.endPos     = (pos_t){LINES, COLS-24};
    versionSelectorWinInfo.prevEndPos = versionSelectorWinInfo.endPos;
    
    customBorder(variantSelectorWin, variantSelectorTitlePtr, &variantSelectorWinInfo);
    customBorder(versionSelectorWin, versionSelectorTitlePtr, &versionSelectorWinInfo);
    mvwaddnstr(mainScreen, 2, 2, endPosText, 8);
    refresh();
    wsyncup(variantSelectorWin);
    wsyncup(versionSelectorWin);
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
    cleanLingeringBorder(win, winInfo);
    winInfo->prevEndPos = winInfo->endPos;

    wmove(win, 0, 0);
    waddch(win, ACS_ULCORNER);
    if(title->len+4 >= winInfo->endPos.col) {
        waddch(win, ACS_HLINE);
        waddch(win, ACS_HLINE);
        waddch(win, ' ');
        waddnstr(win, title->str, title->len);
        waddch(win, ' ');
        for(int i = 6+title->len; i < winInfo->endPos.col; i++) waddch(win, ACS_HLINE);
    }
    else for(int i = 2; i < winInfo->endPos.col; i++) waddch(win, ACS_HLINE);
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

int cleanLingeringBorder(WINDOW* win, winInfo_t* winInfo) {
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