#include "tetris.h"
#include <iostream>
#include <cassert>

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

static char tetromino_grid[8][4][4] =
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

struct Pattern {
    Point loc[4];
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
};

// 8 tetrominoes x 4 rotations
static Pattern tetromino_states[8][4] = {};

// Game logic data
static unsigned long frame = 0;
static bool is_playing = false;
static int period = 60;
static Tetromino tetromino;
static bool debug_mode = false;

static Pattern &get_tetromino_state(int kind, int rotation)
{
    assert(kind >= 0 && kind < B);
    assert(rotation >= 0 && rotation < 4);

    return tetromino_states[kind][rotation];
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
            const char cell = tetromino_grid[kind][y][x];

            if (cell)
                // pattern array index (0, 0) is mapped to Point (-1, -1)
                patt.loc[loc_index++] = rotate({x-1, y-1}, rotation);

            if (loc_index == 4)
                return;
        }
    }
};

bool can_fit(const Tetromino &tet)
{
    for (int i = 0; i < 4; i++) {
        const Point &pos = get_tetromino_state(tet.kind, tet.rotation).loc[i];
        const int field_x = tet.x + pos.x;
        const int field_y = tet.y + pos.y;
        const int field_cell = GetFieldCellKind(field_x, field_y);

        if (field_cell)
            return false;
    }

    return true;
}

void PlayGame()
{
    // Initialize tetrominoes pattern table
    // loop over all tetrominoes
    for (int kind = E; kind < B; kind++)
    {
        // loop over 4 rotations
        for (int rot = 0; rot < 4; rot++) {
            Pattern &patt = get_tetromino_state(kind, rot);
            init_pattern(kind, rot, patt);
        }
    }

    tetromino = Tetromino();
    tetromino.kind = L;
    tetromino.x = 5;
    tetromino.y = 0;

    frame = 1;
    is_playing = true;
}

void QuitGame()
{
    is_playing = false;
}

bool IsPlaying()
{
    return is_playing;
}

void MoveTetromino(int action)
{
    Tetromino moved_tetro = tetromino;

    if (action & MOV_LEFT) {
        moved_tetro.x--;
        if (can_fit(moved_tetro))
            tetromino.x--;
    }
    if (action & MOV_RIGHT) {
        moved_tetro.x++;
        if (can_fit(moved_tetro))
            tetromino.x++;
    }
    if (action & MOV_UP) {
        if (debug_mode) {
            moved_tetro.y--;
            if (can_fit(moved_tetro))
                tetromino.y--;
        }
    }
    if (action & MOV_DOWN) {
        moved_tetro.y++;
        if (can_fit(moved_tetro))
            tetromino.y++;
    }
    if (action & ROT_LEFT) {
        moved_tetro.rotation = (tetromino.rotation - 1 + 4) % 4;
        if (can_fit(moved_tetro))
            tetromino.rotation = moved_tetro.rotation;
    }
    if (action & ROT_RIGHT) {
        moved_tetro.rotation = (tetromino.rotation + 1) % 4;
        if (can_fit(moved_tetro))
            tetromino.rotation = moved_tetro.rotation;
    }
}

void UpdateFrame()
{
    if (frame % period == 0) {
        if (!debug_mode) {
            Tetromino moved_tetro = tetromino;
            moved_tetro.y++;
            if (can_fit(moved_tetro))
                tetromino.y++;
        }
    }

    frame++;
}

Cell GetTetrominoCell(int index)
{
    Cell cell;
    cell.kind = tetromino.kind;

    const Point local = get_tetromino_state(tetromino.kind, tetromino.rotation).loc[index];
    cell.pos.x = tetromino.x + local.x;
    cell.pos.y = tetromino.y + local.y;

    return cell;
}

int GetFieldCellKind(int x, int y)
{
    if (x < 0 || x >= FIELD_WIDTH)
        return B;
    if (y < 0 || y >= FIELD_HEIGHT)
        return B;

    return field[y][x];
}

void SetDebugMode()
{
    debug_mode = true;
}
