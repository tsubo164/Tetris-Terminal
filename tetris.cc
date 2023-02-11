#include "tetris.h"
#include <iostream>
#include <cassert>

Tetris::Tetris()
{
}

Tetris::~Tetris()
{
}

void Tetris::PlayGame()
{
    InitializePieces();

    spawn_tetromino();

    frame = 1;
    is_playing = true;
}

void Tetris::QuitGame()
{
    is_playing = false;
}

bool Tetris::IsPlaying()
{
    return is_playing;
}

void Tetris::MoveTetromino(int action)
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

void Tetris::UpdateFrame()
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

Cell Tetris::GetTetrominoCell(int index)
{
    const Point local = GetPiece(tetromino.kind, tetromino.rotation).cells[index];

    Cell cell;
    cell.kind = tetromino.kind;
    cell.pos = tetromino.pos + local;

    return cell;
}

int Tetris::GetClearingTimer()
{
    return clearing_timer;
}

void Tetris::SetDebugMode()
{
    debug_mode = true;
}

bool Tetris::IsDebugMode()
{
    return debug_mode;
}

void Tetris::ChangeTetrominoKind(int kind)
{
    if (!IsDebugMode())
        return;

    if (!IsSolidCell(kind))
        return;

    tetromino.kind = kind;
}

void Tetris::reset_lock_down_counter()
{
    lock_down_counter = -1;
}

bool Tetris::can_fit(const Tetromino &tet)
{
    for (int i = 0; i < 4; i++) {
        const Point local = GetPiece(tet.kind, tet.rotation).cells[i];
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

bool Tetris::kick_wall(Tetromino &tet, int old_rotation)
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

void Tetris::spawn_tetromino()
{
    static int kind = I;

    tetromino = Tetromino();
    tetromino.kind = kind;
    tetromino.pos = {4, 19};

    kind++;
    if (kind == CELL_END)
        kind = I;
}
