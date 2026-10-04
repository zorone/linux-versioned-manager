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

static void handler(int signum) {
    switch(signum) {
        case SIGWINCH:
            {
                ioctl(0, TIOCGWINSZ, &winsz);
                wresize(mainScreen, winsz.ws_row, winsz.ws_col);
                box(mainScreen, 0, 0);
                refresh();
            }
            break;
        default:
    }
}

int main(int argc, char* argv[]) {
    sigaction_t sa = { .sa_handler=handler };
    sigaction(SIGWINCH, &sa, NULL);

    ioctl(0, TIOCGWINSZ, &winsz);
    
    initscr(); cbreak(); noecho();
    keypad(stdscr, TRUE);

    mainScreen = newwin(winsz.ws_row, winsz.ws_col, 0, 0);
    box(mainScreen, 0, 0);
    refresh();
    wrefresh(mainScreen);
    mainPanel = new_panel(mainScreen);

    while(getch() != 'q');
    endwin();
    return 0;
}