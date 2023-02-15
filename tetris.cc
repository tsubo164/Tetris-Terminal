#include "tetris.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <random>
#include <array>

Tetris::Tetris()
{
}

Tetris::~Tetris()
{
}

void Tetris::PlayGame()
{
    InitializePieces();

    bag_.clear();
    for (int i = 0; i < 2; i++)
        generate_bag();

    spawn_tetromino();

    frame = 1;
    is_playing = true;
}

void Tetris::QuitGame()
{
    is_playing = false;
}

bool Tetris::IsPlaying() const
{
    return is_playing;
}

void Tetris::MoveTetromino(int action)
{
    Tetromino moved_tetro = tetromino_;

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
        tetromino_ = moved_tetro;
        reset_lock_delay_timer();
    }

    // Rot
    if (action & ROT_LEFT)
        moved_tetro.rotation = (tetromino_.rotation - 1 + 4) % 4;

    if (action & ROT_RIGHT)
        moved_tetro.rotation = (tetromino_.rotation + 1) % 4;

    if (kick_wall(moved_tetro, tetromino_.rotation)) {
        tetromino_ = moved_tetro;
        reset_lock_delay_timer();
    }
}

void Tetris::UpdateFrame()
{
    if (clearing_timer > 0) {
        clearing_timer--;
        return;
    }
    else if (clearing_timer == 0) {
        field_.ClearLines();
        clearing_timer = -1;
        spawn_tetromino();
        return;
    }

    if (frame % period == 0) {
        if (IsDebugMode())
            return;

        Tetromino moved_tetro = tetromino_;
        moved_tetro.pos.y--;

        if (can_fit(moved_tetro)) {
            tetromino_.pos.y--;
            reset_lock_delay_timer();
        }
        else if (lock_delay_timer == -1) {
            // start lock down
            lock_delay_timer = playing_fps / 2;
        }
    }

    if (lock_delay_timer == 0) {
        // end lock down
        field_.SetPiece(GetCurrentPiece());
        reset_lock_delay_timer();

        if (GetClearedLineCount() > 0) {
            // start clear lines
            tetromino_.kind = E;
            clearing_timer = 20;
        }
        else {
            // spawn
            spawn_tetromino();
        }
    }
    else if (lock_delay_timer > 0) {
        lock_delay_timer--;
    }

    frame++;
}

bool Tetris::move_piece(int action)
{
    Tetromino &original = tetromino_;
    Tetromino moved = original;
    bool has_moved = false;

    // Gravity drop
    drop_ -= gravity_;

    // Move
    if (action & MOV_LEFT)
        moved.pos.x--;

    if (action & MOV_RIGHT)
        moved.pos.x++;

    if ((action & MOV_UP) && IsDebugMode())
        moved.pos.y++;

    if (action & MOV_DOWN)
        drop_ -= 60./60;

    if (drop_ < -1) {
        moved.pos.y--;
        drop_ += 1;
    }

    if (moved.pos != original.pos)
        has_moved = can_fit(moved);

    // Rot
    if (action & ROT_LEFT)
        moved.rotation = (original.rotation - 1 + 4) % 4;

    if (action & ROT_RIGHT)
        moved.rotation = (original.rotation + 1) % 4;

    if (moved.rotation != original.rotation)
        has_moved = kick_wall(moved, original.rotation);

    // Commit move
    if (has_moved)
        original = moved;

    return has_moved;
}

bool Tetris::has_landed()
{
    Tetromino moved = tetromino_;
    moved.pos.y--;

    return can_fit(moved) == false;
}

void Tetris::UpdateFrame(int action)
{
    const bool moved = move_piece(action);
    const bool landed = has_landed();

    if (moved)
        reset_lock_delay_timer();

    if (landed)
        start_lock_delay_timer();

    if (lock_delay_timer == 0) {
        // end lock down
        reset_lock_delay_timer();
        field_.SetPiece(GetCurrentPiece());

        /*
        if (GetClearedLineCount() > 0) {
            // start clear lines
            tetromino_.kind = E;
            clearing_timer = 20;
        }
        else {
            // spawn
            spawn_tetromino();
        }
        */
        spawn_tetromino();
    }
    else {
        tick_lock_delay_timer();
    }

    /*
    if (clearing_timer > 0) {
        clearing_timer--;
        return;
    }
    else if (clearing_timer == 0) {
        field_.ClearLines();
        clearing_timer = -1;
        spawn_tetromino();
        return;
    }

    if (frame % period == 0) {
        if (IsDebugMode())
            return;

        Tetromino moved_tetro = tetromino_;
        moved_tetro.pos.y--;

        if (can_fit(moved_tetro)) {
            tetromino_.pos.y--;
            reset_lock_delay_timer();
        }
        else if (lock_delay_timer == -1) {
            // start lock down
            lock_delay_timer = playing_fps / 2;
        }
    }

    if (lock_delay_timer == 0) {
        // end lock down
        field_.SetPiece(GetCurrentPiece());
        reset_lock_delay_timer();

        if (GetClearedLineCount() > 0) {
            // start clear lines
            tetromino_.kind = E;
            clearing_timer = 20;
        }
        else {
            // spawn
            spawn_tetromino();
        }
    }
    else if (lock_delay_timer > 0) {
        lock_delay_timer--;
    }
    */

    frame++;
}

int Tetris::GetFieldCellKind(Point pos) const
{
    return field_.GetFieldCellKind(pos);
}

int Tetris::GetClearedLineCount() const
{
    return field_.GetClearedLineCount();
}

void Tetris::GetClearedLines(int *cleared_line_y) const
{
    field_.GetClearedLines(cleared_line_y);
}

int Tetris::GetPieceKindList(int index) const
{
    if (index < 0 || index >= bag_.size())
        return E;

    return bag_[index];
}

Piece Tetris::GetCurrentPiece() const
{
    Piece piece = GetPiece(tetromino_.kind, tetromino_.rotation);

    for (auto &pos: piece.cells)
        pos += tetromino_.pos;

    return piece;
}

Piece Tetris::GetNextPiece(int index) const
{
    const int rotation = 0;
    int kind;

    if (index < 0 || index >= bag_.size())
        kind = E;
    else if (index >= 3)
        kind = E;
    else
        kind = bag_[index];

    return GetPiece(kind, rotation);
}

int Tetris::GetClearingTimer() const
{
    return clearing_timer;
}

void Tetris::SetDebugMode()
{
    debug_mode = true;
}

bool Tetris::IsDebugMode() const
{
    return debug_mode;
}

void Tetris::ChangeTetrominoKind(int kind)
{
    if (!IsDebugMode())
        return;

    if (!IsSolidCell(kind))
        return;

    tetromino_.kind = kind;
}

int Tetris::GetLockDelayTimer() const
{
    return lock_delay_timer;
}

void Tetris::start_lock_delay_timer()
{
    if (lock_delay_timer == -1)
        lock_delay_timer = playing_fps / 2;
}

void Tetris::reset_lock_delay_timer()
{
    lock_delay_timer = -1;
}

void Tetris::tick_lock_delay_timer()
{
    if (lock_delay_timer > 0)
        lock_delay_timer--;
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

    // Pick up offset data based on tetromino_ kind.
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
    if (bag_.size() == 7)
        generate_bag();

    const int kind = bag_.front();
    bag_.pop_front();

    tetromino_ = Tetromino();
    tetromino_.kind = kind;
    tetromino_.pos = {4, 19};
}

void Tetris::generate_bag()
{
    std::array<int, 7> kinds = {I, O, S, Z, J, L, T};
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(kinds.begin(), kinds.end(), rng);

    for (auto kind: kinds)
        bag_.push_back(kind);
}
