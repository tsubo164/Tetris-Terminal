#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

enum TileKind {
    E = 0, // empty
    I,
    O,
    S,
    Z,
    J,
    L,
    T,
    B, // border
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
        {0, 0, 0, 0},
        {I, I, I, I},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // O
        {0, O, O, 0},
        {0, O, O, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // S
        {0, S, S, 0},
        {S, S, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // Z
        {Z, Z, 0, 0},
        {0, Z, Z, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // J
        {J, 0, 0, 0},
        {J, J, J, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // L
        {0, 0, L, 0},
        {L, L, L, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // T
        {0, T, 0, 0},
        {T, T, T, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
};

struct Point {
    Point() : x(0), y(0) {}
    Point(int xx, int yy) : x(xx), y(yy) {}
    int x, y;
};

struct Pattern {
    Point loc[4];
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
} tetro = {0};

// 8 tetrominoes x 4 rotations
static Pattern tetromino_pattern[8][4] = {};

static void init_pattern(int kind, int rotation, Pattern &patt);
static int rotate(int rot, int dir);
static bool can_fit(const Tetromino &tet);
static void render();

int main(int argc, char **argv)
{
    // Initialize tetrominoes pattern table
    {
        // loop over all tetrominoes
        for (int kind = E; kind < B; kind++)
        {
            // loop over 4 rotations
            for (int rot = 0; rot < 4; rot++) {
                Pattern &patt = tetromino_pattern[kind][rot];

                init_pattern(kind, rot, patt);

                printf("rot %d =================\n", rot);
                for (int j = 0; j < 4; j++) {
                    printf("(%d, %d)\n", patt.loc[j].x, patt.loc[j].y);
                }
            }
        }
    }

    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    unsigned long frame = 0;
    double fps = 0.0;
    auto start = std::chrono::steady_clock::now();

    tetro.kind = Z;
    tetro.x = 5;
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
            /*
            moved_tetro = tetro;
            moved_tetro.y++;
            if (can_fit(moved_tetro))
                tetro.y++;
            */
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
    for (int i = 0; i < 4; i++) {
        const Point &pos = tetromino_pattern[tet.kind][tet.rotation].loc[i];
        const char sym = get_tile_symbol(tet.kind);
        draw_char(tet.x + pos.x, tet.y + pos.y, sym);
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

static Point rotate(Point point, int rotation)
{
    if (rotation < 1 || rotation > 4)
        return point;

    Point rotated = point;

    for (int i = 0; i < rotation; i++) {
        const Point tmp = {-rotated.y, rotated.x};
        rotated = tmp;
    }

    return rotated;
}

void init_pattern(int kind, int rotation, Pattern &patt)
{
    int loc_index = 0;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            const char tile = tetromino[kind][y][x];

            if (tile)
                // pattern array index (0, 0) is mapped to Point (-1, -1)
                patt.loc[loc_index++] = rotate(Point(x-1, y-1), rotation);

            if (loc_index == 4)
                return;
        }
    }
};

static int rotate(int rot, int dir)
{
    if (dir != -1 && dir != 1)
        return rot;

    return (tetro.rotation + dir + 4) % 4;
}

bool can_fit(const Tetromino &tet)
{
    for (int i = 0; i < 4; i++) {
        const Point &pos = tetromino_pattern[tet.kind][tet.rotation].loc[i];
        const int field_x = tet.x + pos.x;
        const int field_y = tet.y + pos.y;
        const int field_tile = get_field_tile(field_x, field_y);

        if (field_tile)
            return false;
    }

    return true;
}
