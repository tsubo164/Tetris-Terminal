#include "tetris.h"
#include <iostream>

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

struct Pattern {
    Point loc[4];
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
};

// 8 tetrominoes x 4 rotations
static Pattern tetromino_pattern[8][4] = {};

// Game logic data
static unsigned long frame = 0;
static bool is_playing = false;
static int period = 60;
static Tetromino tetro;

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
            const char cell = tetromino[kind][y][x];

            if (cell)
                // pattern array index (0, 0) is mapped to Point (-1, -1)
                patt.loc[loc_index++] = rotate(Point(x-1, y-1), rotation);

            if (loc_index == 4)
                return;
        }
    }
};

bool can_fit(const Tetromino &tet)
{
    for (int i = 0; i < 4; i++) {
        const Point &pos = tetromino_pattern[tet.kind][tet.rotation].loc[i];
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
            Pattern &patt = tetromino_pattern[kind][rot];

            init_pattern(kind, rot, patt);

            printf("rot %d =================\n", rot);
            for (int j = 0; j < 4; j++) {
                printf("(%d, %d)\n", patt.loc[j].x, patt.loc[j].y);
            }
        }
    }

    tetro.kind = Z;
    tetro.x = 5;
    tetro.y = 0;

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
    Tetromino moved_tetro = tetro;

    if (action & MOV_LEFT) {
        moved_tetro.x--;
        if (can_fit(moved_tetro))
            tetro.x--;
    }
    if (action & MOV_RIGHT) {
        moved_tetro.x++;
        if (can_fit(moved_tetro))
            tetro.x++;
    }
    if (action & MOV_UP) {
        moved_tetro.y--;
        if (can_fit(moved_tetro))
            tetro.y--;
    }
    if (action & MOV_DOWN) {
        moved_tetro.y++;
        if (can_fit(moved_tetro))
            tetro.y++;
    }
    if (action & ROT_LEFT) {
        moved_tetro.rotation = (tetro.rotation - 1 + 4) % 4;
        if (can_fit(moved_tetro))
            tetro.rotation = moved_tetro.rotation;
    }
    if (action & ROT_RIGHT) {
        moved_tetro.rotation = (tetro.rotation + 1) % 4;
        if (can_fit(moved_tetro))
            tetro.rotation = moved_tetro.rotation;
    }
}

void UpdateFrame()
{
    if (frame % period == 0) {
        Tetromino moved_tetro = tetro;
        moved_tetro.y++;
        if (can_fit(moved_tetro))
            tetro.y++;
    }

    frame++;
}

Cell GetTetrominoCell(int index)
{
    Cell cell;
    cell.kind = tetro.kind;

    const Point local = tetromino_pattern[tetro.kind][tetro.rotation].loc[index];
    cell.pos.x = tetro.x + local.x; 
    cell.pos.y = tetro.y + local.y; 

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
