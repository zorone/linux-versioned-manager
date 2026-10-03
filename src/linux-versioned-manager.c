#ifdef __MINGW32__
    #include <ncurses/ncurses.h>
    #include <ncurses/panel.h>
#else
    #include <ncurses.h>
    #include <panel.h>
#endif

#include <stdio.h>

int main(int argc, char* argv[]) {
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