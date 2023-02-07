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

struct RotationState {
    Point cells[4];
};

struct Tetromino {
    int kind;
    int x, y;
    int rotation;
};

// 8 tetrominoes x 4 rotations
static RotationState rotation_states[8][4] = {};

// Game logic data
static unsigned long frame = 0;
static bool is_playing = false;
static int period = 60;
static Tetromino tetromino;
static bool debug_mode = false;

static RotationState &get_rotaion_state(int kind, int rotation)
{
    assert(kind >= 0 && kind < B);
    assert(rotation >= 0 && rotation < 4);

    return rotation_states[kind][rotation];
}

static Point rotate(Point point, int rotation)
{
    if (rotation < 1 || rotation > 4)
        return point;

    Point result = point;

    for (int i = 0; i < rotation; i++) {
        const Point tmp = {-result.y, result.x};
        result = tmp;
    }

    return result;
}

static void init_state(int kind, int rotation, RotationState &state)
{
    int cell_index = 0;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            const char cell = tetromino_grid[kind][y][x];

            if (cell) {
                // state array index (0, 0) is mapped to Point (-1, -1)
                Point p = rotate({x - 1, y - 1}, rotation);
                state.cells[cell_index++] = p;
            }

            if (cell_index == 4)
                return;
        }
    }
};

static void initialize_rotation_states()
{
    // loop over all tetrominoes
    for (int kind = E; kind < B; kind++)
    {
        // loop over 4 rotations
        for (int rot = 0; rot < 4; rot++) {
            RotationState &state = get_rotaion_state(kind, rot);
            init_state(kind, rot, state);
        }
    }
}

static bool can_fit(const Tetromino &tet)
{
    for (int i = 0; i < 4; i++) {
        const Point &pos = get_rotaion_state(tet.kind, tet.rotation).cells[i];
        const int field_x = tet.x + pos.x;
        const int field_y = tet.y + pos.y;
        const int field_cell = GetFieldCellKind(field_x, field_y);

        if (field_cell)
            return false;
    }

    return true;
}

// TTC offset data is based on Y up. So Y values in this table
// have been nagated from their's
// https://tetris.wiki/Super_Rotation_System
static const Point offset_table_jlstz [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {+1, 0}, {+1,+1}, { 0,-2}, {+1,-2}},
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {-1, 0}, {-1,+1}, { 0,-2}, {-1,-2}},
};
static const Point offset_table_i [4][5] = {
    {{ 0, 0}, {-1, 0}, {+2, 0}, {-1, 0}, {+2, 0}},
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0,-1}, { 0,+2}},
    {{-1,-1}, {+1,-1}, {-2,-1}, {+1, 0}, {-2, 0}},
    {{ 0,-1}, { 0,-1}, { 0,-1}, { 0,+1}, { 0,-2}},
};
static const Point offset_table_o [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0,+1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{-1,+1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
};

static bool kick_wall(Tetromino &tet, int old_rotation)
{
    const int new_rotation = tet.rotation;
    const Point *offsets0 = nullptr;
    const Point *offsets1 = nullptr;

    // Pick up offset data based on tetromino kind.
    if (tet.kind == I) {
        offsets0 = offset_table_i[old_rotation];
        offsets1 = offset_table_i[new_rotation];
    }
    else if (tet.kind == O) {
        offsets0 = offset_table_o[old_rotation];
        offsets1 = offset_table_o[new_rotation];
    }
    else {
        offsets0 = offset_table_jlstz[old_rotation];
        offsets1 = offset_table_jlstz[new_rotation];
    }

    // Tests against 5 offsets.
    for (int i = 0; i < 5; i++) {
        const Point offset0 = offsets0[i];
        const Point offset1 = offsets1[i];

        Tetromino test = tet;
        test.x += offset0.x - offset1.x;
        test.y += offset0.y - offset1.y;

        if (can_fit(test)) {
            tet.x = test.x;
            tet.y = test.y;
            return true;
        }
    }
    return false;
}

void PlayGame()
{
    initialize_rotation_states();

    tetromino = Tetromino();
    tetromino.kind = I;
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
        if (IsDebugMode()) {
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
        if (kick_wall(moved_tetro, tetromino.rotation)) {
            tetromino.rotation = moved_tetro.rotation;
            tetromino.x = moved_tetro.x;
            tetromino.y = moved_tetro.y;
        }
    }
    if (action & ROT_RIGHT) {
        moved_tetro.rotation = (tetromino.rotation + 1) % 4;
        if (kick_wall(moved_tetro, tetromino.rotation)) {
            tetromino.rotation = moved_tetro.rotation;
            tetromino.x = moved_tetro.x;
            tetromino.y = moved_tetro.y;
        }
    }
}

void UpdateFrame()
{
    if (frame % period == 0) {
        if (!IsDebugMode()) {
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

    const Point local = get_rotaion_state(tetromino.kind, tetromino.rotation).cells[index];
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

bool IsDebugMode()
{
    return debug_mode;
}

void ChangeTetrominoKind(int kind)
{
    if (!IsDebugMode())
        return;

    if (kind <= E || kind >= B)
        return;

    tetromino.kind = kind;
}
