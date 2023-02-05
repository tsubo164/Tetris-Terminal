#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

enum TileKind {
    E = 0, // empty
    I,     // tetromino
    O,     // tetromino
    S,     // tetromino
    Z,     // tetromino
    J,     // tetromino
    L,     // tetromino
    T,     // tetromino
    B,     // border
};

static constexpr int FIELD_WIDTH = 10 + 2;
static constexpr int FIELD_HEIGHT = 20 + 2;

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

static char tetromino[8][4][4] =
{
    { // E
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // I
        {0, I, 0, 0},
        {0, I, 0, 0},
        {0, I, 0, 0},
        {0, I, 0, 0},
    },
    { // O
        {0, 0, 0, 0},
        {0, O, O, 0},
        {0, O, O, 0},
        {0, 0, 0, 0},
    },
    { // S
        {0, S, 0, 0},
        {0, S, S, 0},
        {0, 0, S, 0},
        {0, 0, 0, 0},
    },
    { // Z
        {0, 0, Z, 0},
        {0, Z, Z, 0},
        {0, Z, 0, 0},
        {0, 0, 0, 0},
    },
    { // J
        {0, 0, J, 0},
        {0, 0, J, 0},
        {0, J, J, 0},
        {0, 0, 0, 0},
    },
    { // L
        {0, L, 0, 0},
        {0, L, 0, 0},
        {0, L, L, 0},
        {0, 0, 0, 0},
    },
    { // T
        {0, 0, 0, 0},
        {0, T, T, T},
        {0, 0, T, 0},
        {0, 0, 0, 0},
    },
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
} tetro = {0};

static int rotate(int rot, int dir);
static int lookup_tetromino(const Tetromino &tet, int u, int v);
static bool can_fit(const Tetromino &tet);
static void render();

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    unsigned long frame = 0;
    double fps = 0.0;
    auto start = std::chrono::steady_clock::now();

    tetro.kind = T;
    tetro.x = 4;
    tetro.y = 0;

    bool is_playing = true;
    int period = 60;

    while (is_playing) {

        // Input
        const int key = getch();

        // Game logic
        Tetromino moved_tetro = tetro;

        switch (key) {

        case 'd':
            moved_tetro.rotation = rotate(tetro.rotation, -1);
            if (can_fit(moved_tetro))
                tetro.rotation = moved_tetro.rotation;
            break;

        case 'f':
            moved_tetro.rotation = rotate(tetro.rotation, 1);
            if (can_fit(moved_tetro))
                tetro.rotation = moved_tetro.rotation;
            break;

        case 'h':
            moved_tetro.x--;
            if (can_fit(moved_tetro))
                tetro.x--;
            break;

        case 'l':
            moved_tetro.x++;
            if (can_fit(moved_tetro))
                tetro.x++;
            break;

        case 'k':
            moved_tetro.y--;
            if (can_fit(moved_tetro))
                tetro.y--;
            break;

        case 'j':
            moved_tetro.y++;
            if (can_fit(moved_tetro))
                tetro.y++;
            break;

        case 'q':
            is_playing = false;
            break;

        default:
            break;
        }

        if (frame % period == 0) {
            moved_tetro = tetro;
            moved_tetro.y++;
            if (can_fit(moved_tetro))
                tetro.y++;
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
        frame++;

        if (frame % 10 == 0)
            fps = 1. / elapsed_sec;
    }

    endwin();

    return 0;
}

static void draw_char(int x, int y, char ch)
{
    mvaddch(y, x, ch);
}

static void draw_str(int x, int y, const char *str)
{
    mvaddstr(y, x, str);
}

static int get_field_tile(int x, int y)
{
    if (x < 0 || x >= FIELD_WIDTH)
        return B;
    if (y < 0 || y >= FIELD_HEIGHT)
        return B;

    return field[y][x];
}

static char get_tile_symbol(int tile)
{
    switch (tile) {
        case E: return ' ';
        case I: return 'I';
        case O: return 'O';
        case S: return 'S';
        case Z: return 'Z';
        case J: return 'J';
        case L: return 'L';
        case T: return 'T';
        case B: return 'B';
        default: return ' ';
    }
}

static void draw_tetromino(const Tetromino &tet)
{
    for (int v = 0; v < 4; v++) {
        for (int u = 0; u < 4; u++) {
            const int tile = lookup_tetromino(tet, u, v);
            if (!tile)
                continue;

            const char sym = get_tile_symbol(tile);
            draw_char(tet.x + u, tet.y + v, sym);
        }
    }
}

void render()
{
    char line[FIELD_WIDTH + 1] = {'\0'};

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int tile = get_field_tile(x, y);
            line[x] = get_tile_symbol(tile);
        }
        draw_str(0, y, line);
    }

    draw_tetromino(tetro);
}

static int rotate(int rot, int dir)
{
    if (dir != -1 && dir != 1)
        return rot;

    return (tetro.rotation + dir + 4) % 4;
}

int lookup_tetromino(const Tetromino &tet, int u, int v)
{
    struct Coord { int u, v; };

    static const Coord lookup_table[4][4][4] = {
        { // rot 0
            {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
            {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
            {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
            {{0, 3}, {1, 3}, {2, 3}, {3, 3}}
        },
        { // rot 1
            {{0, 3}, {0, 2}, {0, 1}, {0, 0}},
            {{1, 3}, {1, 2}, {1, 1}, {1, 0}},
            {{2, 3}, {2, 2}, {2, 1}, {2, 0}},
            {{3, 3}, {3, 2}, {3, 1}, {3, 0}}
        },
        { // rot 2
            {{3, 3}, {2, 3}, {1, 3}, {0, 3}},
            {{3, 2}, {2, 2}, {1, 2}, {0, 2}},
            {{3, 1}, {2, 1}, {1, 1}, {0, 1}},
            {{3, 0}, {2, 0}, {1, 0}, {0, 0}}
        },
        { // rot 3
            {{3, 0}, {3, 1}, {3, 2}, {3, 3}},
            {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
            {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
            {{0, 0}, {0, 1}, {0, 2}, {0, 3}}
        }
    };

    const Coord coord = lookup_table[tet.rotation][v][u];
    return tetromino[tet.kind][coord.v][coord.u];
}

bool can_fit(const Tetromino &tet)
{
    for (int v = 0; v < 4; v++) {
        for (int u = 0; u < 4; u++) {
            const int tet_tile = lookup_tetromino(tet, u, v);

            if (!tet_tile)
                continue;

            const int field_x = tet.x + u;
            const int field_y = tet.y + v;
            const int field_tile = get_field_tile(field_x, field_y);

            if (field_tile)
                return false;
        }
    }

    return true;
}
