#include <iostream>
#include <chrono>
#include <string>
#include <ncurses.h>

enum TileKind {
    E = 0, // empty
    B,     // border
    I,     // tetromino
    O,     // tetromino
    S,     // tetromino
    Z,     // tetromino
    J,     // tetromino
    L,     // tetromino
    T,     // tetromino
};

static const int FIELD_WIDTH = 10 + 2;
static const int FIELD_HEIGHT = 20 + 2;

static char field[FIELD_HEIGHT][FIELD_WIDTH] =
{
    {B,B,B,0,0,0,0,0,0,B,B,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,B,B,B,B,B,B,B,B,B,B,B},
};

static char tetromino[4][4] =
{
    {0, 0, S, 0},
    {0, S, S, 0},
    {0, S, 0, 0},
    {0, 0, 0, 0},
};

static void render();

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    int frame = 0;
    double fps = 0.0;
    auto start = std::chrono::steady_clock::now();

    while (1) {
        clear();

        const std::string msg = "frame: " + std::to_string(frame);
        const std::string fpsstr = "fps: " + std::to_string(fps);

        mvaddstr( 9, 20, fpsstr.c_str());
        mvaddstr(10, 20, msg.c_str());
        mvaddstr(11, 20, "Press 'Q' to quit");

        render();;

        refresh();

        const int c = getch();
        if (c == 'q')
            break;

        const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - start;
        if (dur.count() > .1) {
            start = std::chrono::steady_clock::now();
            fps = frame / dur.count();
            frame = 0;
        }
        else {
            frame++;
        }
    }

    endwin();

    return 0;
}

static void draw_str(int pos_x, int pos_y, const char *str)
{
    mvaddstr(pos_y, pos_x, str);
}

static int get_tile(int pos_x, int pos_y)
{
    return field[pos_y][pos_x];
}

static void set_tile(int pos_x, int pos_y, int kind)
{
    field[pos_y][pos_x] = kind;
}

static void draw_tetromino(int pos_x, int pos_y, int kind)
{
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            const int tile = tetromino[y][x];
            if (tile)
                set_tile(pos_x + x, pos_y + y, tile);
        }
    }
}

void render()
{
    char line[FIELD_WIDTH + 1] = {'\0'};

    draw_tetromino(4, 12, S);

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int tile = get_tile(x, y);
            char ch = ' ';

            switch (tile) {
                case E: ch = ' '; break;
                case B: ch = 'B'; break;
                case S: ch = 'S'; break;
                default: break;
            }

            line[x] = ch;
        }
        draw_str(0, y, line);
    }
}
