#include <iostream>
#include <string>
#include <ncurses.h>

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    int frame = 0;

    while (1) {
        clear();

        const std::string msg = "frame: " + std::to_string(frame);
        mvaddstr(10, 20, msg.c_str());
        mvaddstr(11, 20, "Press 'Q' to quit");

        refresh();

        const int c = getch();
        if (c == 'q')
            break;

        frame++;
    }

    endwin();

    return 0;
}
