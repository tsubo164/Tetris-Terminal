#include "tetris.h"
#include <iostream>
#include <cassert>

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
    int rotation;
    Point pos;
};

// 8 tetrominoes x 4 rotations
static RotationState rotation_states[8][4] = {};

// Game logic data
static unsigned long frame = 0;
static bool is_playing = false;
static int period = 60;
static Tetromino tetromino;
static bool debug_mode = false;

static int playing_fps = 60;
static int lock_down_counter = -1;

static int clearing_timer = -1;

static void reset_lock_down_counter()
{
    lock_down_counter = -1;
}

static RotationState &get_rotaion_state(int kind, int rotation)
{
    assert(IsValidCell(kind));
    assert(rotation >= 0 && rotation < 4);

    return rotation_states[kind][rotation];
}

static Point rotate(Point point, int rotation)
{
    if (rotation < 1 || rotation > 4)
        return point;

    Point result = point;

    for (int i = 0; i < rotation; i++) {
        const Point tmp = {result.y, -result.x};
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
                // local = grid(x, invert(y)) - center
                Point local = Point(x, 4 - y - 1) - Point(1, 2);
                local = rotate(local, rotation);

                state.cells[cell_index++] = local;
            }

            if (cell_index == 4)
                return;
        }
    }
};

static void initialize_rotation_states()
{
    // loop over all tetrominoes
    for (int kind = E; kind < CELL_END; kind++)
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
        const Point local = get_rotaion_state(tet.kind, tet.rotation).cells[i];
        const Point field = tet.pos + local;
        const int field_cell = GetFieldCellKind(field);

        if (field_cell)
            return false;
    }

    return true;
}

// TTC's super rotation system.
// https://tetris.wiki/Super_Rotation_System
static const Point offset_table_jlstz [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {+1, 0}, {+1,-1}, { 0,+2}, {+1,+2}},
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {-1, 0}, {-1,-1}, { 0,+2}, {-1,+2}},
};
static const Point offset_table_i [4][5] = {
    {{ 0, 0}, {-1, 0}, {+2, 0}, {-1, 0}, {+2, 0}},
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0,+1}, { 0,-2}},
    {{-1,+1}, {+1,+1}, {-2,+1}, {+1, 0}, {-2, 0}},
    {{ 0,+1}, { 0,+1}, { 0,+1}, { 0,-1}, { 0,+2}},
};
static const Point offset_table_o [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0,-1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{-1,-1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
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
        test.pos += offset0 - offset1;

        if (can_fit(test)) {
            tet.pos = test.pos;
            return true;
        }
    }
    return false;
}

static void spawn_tetromino()
{
    static int kind = I;

    tetromino = Tetromino();
    tetromino.kind = kind;
    tetromino.pos = {4, 19};

    kind++;
    if (kind == CELL_END)
        kind = I;
}

void PlayGame()
{
    initialize_rotation_states();

    spawn_tetromino();

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

    // Move
    if (action & MOV_LEFT)
        moved_tetro.pos.x--;

    if (action & MOV_RIGHT)
        moved_tetro.pos.x++;

    if (action & MOV_DOWN)
        moved_tetro.pos.y--;

    if ((action & MOV_UP) && IsDebugMode())
        moved_tetro.pos.y++;

    if (can_fit(moved_tetro)) {
        tetromino = moved_tetro;
        reset_lock_down_counter();
    }

    // Rot
    if (action & ROT_LEFT)
        moved_tetro.rotation = (tetromino.rotation - 1 + 4) % 4;

    if (action & ROT_RIGHT)
        moved_tetro.rotation = (tetromino.rotation + 1) % 4;

    if (kick_wall(moved_tetro, tetromino.rotation)) {
        tetromino = moved_tetro;
        reset_lock_down_counter();
    }
}

void UpdateFrame()
{
    if (clearing_timer > 0) {
        clearing_timer--;
        return;
    }
    else if (clearing_timer == 0) {
        ClearLines();
        clearing_timer = -1;
        spawn_tetromino();
        return;
    }

    if (frame % period == 0) {
        if (IsDebugMode())
            return;

        Tetromino moved_tetro = tetromino;
        moved_tetro.pos.y--;

        if (can_fit(moved_tetro)) {
            tetromino.pos.y--;
            reset_lock_down_counter();
        }
        else if (lock_down_counter == -1) {
            // start lock down
            lock_down_counter = playing_fps / 2;
        }
    }

    if (lock_down_counter == 0) {
        // end lock down
        for (int i = 0; i < 4; i++) {
            const Cell cell = GetTetrominoCell(i);
            SetFieldCellKind(cell.pos, cell.kind);
        }
        reset_lock_down_counter();

        if (GetClearedLineCount() > 0) {
            // clear lines
            tetromino.kind = E;
            clearing_timer = 20;
        }
        else {
            // spawn
            spawn_tetromino();
        }
    }
    else if (lock_down_counter > 0) {
        lock_down_counter--;
    }

    frame++;
}

Cell GetTetrominoCell(int index)
{
    const Point local = get_rotaion_state(tetromino.kind, tetromino.rotation).cells[index];

    Cell cell;
    cell.kind = tetromino.kind;
    cell.pos = tetromino.pos + local;

    return cell;
}

int GetClearingTimer()
{
    return clearing_timer;
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

    if (!IsSolidCell(kind))
        return;

    tetromino.kind = kind;
}
