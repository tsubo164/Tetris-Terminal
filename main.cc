#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

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
    {0, S, 0, 0},
    {0, S, S, 0},
    {0, 0, S, 0},
    {0, 0, 0, 0},
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
} tetro = {0};

static int lookup_tetromino(const Tetromino &tetro, int x, int y);
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

    tetro.x = 4;
    tetro.y = 0;

    bool is_playing = true;

    while (is_playing) {

        // Input
        const int key = getch();

        // Game logic
        switch (key) {

        case 'd':
            tetro.rotation = (tetro.rotation - 1 + 4) % 4;
            break;

        case 'f':
            tetro.rotation = (tetro.rotation + 1) % 4;
            break;

        case 'h':
            tetro.x--;
            break;

        case 'l':
            tetro.x++;
            break;

        case 'k':
            tetro.y--;
            break;

        case 'j':
            tetro.y++;
            break;

        case 'q':
            is_playing = false;
            break;

        default:
            break;
        }

        // Rednering
        erase();

        render();;

        const std::string fpsstr = "fps: " + std::to_string(fps);
        mvaddstr(10, 20, fpsstr.c_str());
        mvaddstr(11, 20, "Press 'Q' to quit");

        refresh();

        // Measure frame cost
        const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - start;
        const double elapsed_sec = dur.count();
        const long remaining_millisec = 1000 * (1./60 - elapsed_sec);

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(remaining_millisec));

        // Restart timer
        start = std::chrono::steady_clock::now();
        fps = 1. / elapsed_sec;
        frame++;
    }

    endwin();

    return 0;
}

static void draw_char(int pos_x, int pos_y, char ch)
{
    mvaddch(pos_y, pos_x, ch);
}

static void draw_str(int pos_x, int pos_y, const char *str)
{
    mvaddstr(pos_y, pos_x, str);
}

static int get_tile(int pos_x, int pos_y)
{
    return field[pos_y][pos_x];
}

static void draw_tetromino(const Tetromino &tetro)
{
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            //const int tile = tetromino[y][x];
            const int tile = lookup_tetromino(tetro, x, y);
            char ch = '\0';

            switch (tile) {
                case B: ch = 'B'; break;
                case S: ch = 'S'; break;
                default: continue;
            }

            draw_char(tetro.x + x, tetro.y + y, ch);
        }
    }
}

void render()
{
    char line[FIELD_WIDTH + 1] = {'\0'};

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

    draw_tetromino(tetro);
}

static int lookup_tetromino(const Tetromino &tet, int x, int y)
{
    struct Coord { int x, y; };

    static const Coord lookup_table[4][4][4] = {
        {
            // rot 0
            {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
            {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
            {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
            {{0, 3}, {1, 3}, {2, 3}, {3, 3}}
        },
        {
            // rot 1
            {{0, 3}, {0, 2}, {0, 1}, {0, 0}},
            {{1, 3}, {1, 2}, {1, 1}, {1, 0}},
            {{2, 3}, {2, 2}, {2, 1}, {2, 0}},
            {{3, 3}, {3, 2}, {3, 1}, {3, 0}}
        },
        {
            // rot 2
            {{3, 3}, {2, 3}, {1, 3}, {0, 3}},
            {{3, 2}, {2, 2}, {1, 2}, {0, 2}},
            {{3, 1}, {2, 1}, {1, 1}, {0, 1}},
            {{3, 0}, {2, 0}, {1, 0}, {0, 0}}
        },
        {
            // rot 3
            {{3, 0}, {3, 1}, {3, 2}, {3, 3}},
            {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
            {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
            {{0, 0}, {0, 1}, {0, 2}, {0, 3}}
        }
    };

    const Coord coord = lookup_table[tet.rotation][y][x];
    return tetromino[coord.y][coord.x];
}
