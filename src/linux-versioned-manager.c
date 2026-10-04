#include <asm-generic/ioctls.h>
#include <stdio.h>
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

WINDOW *mainScreen = NULL;
PANEL *mainPanel = NULL;

char displayText[8] = ""; 

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