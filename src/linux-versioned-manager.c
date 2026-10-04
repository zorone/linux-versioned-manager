#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

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
int customBorder(WINDOW* win, int, pos_t start, pos_t end);

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

    mainScreen = newwin(LINES, COLS, 0, 0);
    box(mainScreen, 0, 0);
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

int customBorder(WINDOW* win, int, pos_t start, pos_t end) {
    mvwprintw(win, 0, 0, "┌%*s┐", end.col-1, "─");
    wmove(win, 1, 0);
    
}
// ┌─┐
// │ │
// └─┘