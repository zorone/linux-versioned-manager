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

static void handler(int signum) {

}

int main(int argc, char* argv[]) {
    sigaction_t sa = { .sa_handler=handler };
    sigaction(SIGWINCH, &sa, NULL);

    winsize_t winsz;
    initscr(); cbreak(); noecho();
    keypad(stdscr, TRUE);

    WINDOW *mainScreen = newwin(50, 50, 1, 1);
    box(mainScreen, 0, 0);
    refresh();
    wrefresh(mainScreen);
    PANEL *mainPanel = new_panel(mainScreen);

    getch();
    endwin();
    return 0;
}