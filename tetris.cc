#include "tetris.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <random>
#include <array>

static float get_gravity(int level)
{
    static const float gravity_table[21] = {
        // level 0 - 20
        0.0, 0.01667, 0.021017, 0.026977, 0.035256, 0.04693, 0.06361, 0.0879, 0.1236,
        0.1775, 0.2598, 0.388, 0.59, 0.92, 1.46, 2.36, 3.91, 6.61, 11.43, 20,
    };

    if (level < 0)
        return gravity_table[0];
    else if (level > 20)
        return gravity_table[20];
    else
        return gravity_table[level];
}

static int get_score(int action)
{
    switch (action) {
    case 1: return 100;
    case 2: return 300;
    case 3: return 500;
    case 4: return 800;
    default: return 0;
    };
}

Tetris::Tetris()
{
}

Tetris::~Tetris()
{
}

void Tetris::PlayGame()
{
    // Pieces
    InitializePieces();

    // Bags
    bag_.clear();
    for (int i = 0; i < 2; i++)
        generate_bag();

    // Field
    field_.Clear();
    field_.SetTopHole(2, 7);

    // Game
    is_playing_ = true;
    is_game_over_ = false;
    frame_ = 1;

    gravity_ = get_gravity(1);
    total_line_count_ = 0;
    level_ = 1;
    score_ = 0;

    // Start
    spawn_tetromino();
}

void Tetris::QuitGame()
{
    is_playing_ = false;
}

void Tetris::PauseGame()
{
    is_paused_ = !is_paused_;
}

bool Tetris::IsPlaying() const
{
    return is_playing_;
}

bool Tetris::IsGameOver() const
{
    return is_game_over_;
}

bool Tetris::IsPaused() const
{
    return is_paused_;
}

void Tetris::SetPreviewCount(int count)
{
    preview_count_ = std::min(std::max(1, count), 6);
}

bool Tetris::drop_piece(Tetromino &tet)
{
    Tetromino &current = tet;
    Tetromino moved = tet;
    bool has_moved = false;

    do {
        moved.pos.y--;
    } while (moved.CanFit(field_));

    has_moved = ++moved.pos.y != current.pos.y;
    if (has_moved)
        current = moved;

    return has_moved;
}

bool Tetris::move_piece(int action)
{
    Tetromino &current = tetromino_;
    Tetromino moved = tetromino_;
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

    if (moved.pos != current.pos)
        has_moved = moved.CanFit(field_);
    // TODO commit move before rotation
    // there might be a situation it can move but not rotate

    // Rot
    if (action & ROT_LEFT)
        moved.rotation = (current.rotation - 1 + 4) % 4;

    if (action & ROT_RIGHT)
        moved.rotation = (current.rotation + 1) % 4;

    if (moved.rotation != current.rotation)
        has_moved = moved.KickWall(field_, current.rotation);

    // Commit move
    if (has_moved)
        current = moved;

    return has_moved;
}

bool Tetris::has_landed()
{
    Tetromino moved = tetromino_;
    moved.pos.y--;

    return moved.CanFit(field_) == false;
}

void Tetris::UpdateFrame(int action)
{
    if (IsGameOver() || IsPaused())
        return;

    // Clears lines
    if (clearing_timer_ > 0) {
        clearing_timer_--;
        return;
    }
    else if (clearing_timer_ == 0) {
        // lines, level, gravity
        const int count = field_.GetClearedLineCount();
        total_line_count_ += count;
        if (total_line_count_ >= 5 * level_)
            level_++;
        gravity_ = get_gravity(level_);
        score_ += get_score(count);

        field_.ClearLines();
        spawn_tetromino();
        return;
    }

    // Actions
    if (action & HOLD_PIECE) {
        if (IsEmptyCell(hold_.kind)) {
            hold_ = tetromino_;
            hold_.rotation = 0;
            hold_.pos = {};
            spawn_tetromino();
            return;
        }
        else if (IsHoldAvailable()) {
            std::swap(tetromino_, hold_);
            tetromino_.pos = {4, 19};
            hold_.rotation = 0;
            hold_.pos = {0, 0};
            // Timers and counter
            lock_delay_timer_ = -1;
            clearing_timer_ = -1;
            reset_counter_ = 0;
            is_hold_available_ = false;
            return;
        }
    }
    else if (action & MOV_HARDDROP) {
        drop_piece(tetromino_);
        lock_delay_timer_ = 0;
    }
    else {
        const bool moved = move_piece(action);

        if (moved) {
            if (lock_delay_timer_ > 0 && reset_counter_ < 16) {
                reset_lock_delay_timer();
                reset_counter_++;
            }
        }
    }

    // Ghost
    ghost_ = tetromino_;
    drop_piece(ghost_);
    if (ghost_.pos == tetromino_.pos)
        ghost_.kind = E;

    bool landed = has_landed();
    if (landed)
        start_lock_delay_timer();

    if (lock_delay_timer_ == 0 && landed) {
        field_.SetPiece(GetCurrentPiece());

        if (GetClearedLineCount() > 0) {
            // start clear lines
            tetromino_.kind = E;
            clearing_timer_ = 20;
        }
        else {
            // spawn
            spawn_tetromino();
        }
    }
    else {
        tick_lock_delay_timer();
    }

    frame_++;
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

Piece Tetris::GetGhostPiece() const
{
    Piece piece = GetPiece(ghost_.kind, ghost_.rotation);

    for (auto &pos: piece.cells)
        pos += ghost_.pos;

    return piece;
}

Piece Tetris::GetHoldPiece() const
{
    Piece piece = GetPiece(hold_.kind, hold_.rotation);

    for (auto &pos: piece.cells)
        pos += hold_.pos;

    return piece;
}

Piece Tetris::GetNextPiece(int index) const
{
    const int rotation = 0;
    int kind;

    if (index < 0 || index >= bag_.size())
        kind = E;
    else if (index >= preview_count_)
        kind = E;
    else
        kind = bag_[index];

    return GetPiece(kind, rotation);
}

bool Tetris::IsHoldAvailable() const
{
    return is_hold_available_;
}

int Tetris::GetLevel() const
{
    return level_;
}

int Tetris::GetScore() const
{
    return score_;
}

int Tetris::GetTotalLineCount() const
{
    return total_line_count_;
}

int Tetris::GetClearingTimer() const
{
    return clearing_timer_;
}

void Tetris::SetDebugMode()
{
    debug_mode_ = true;
}

bool Tetris::IsDebugMode() const
{
    return debug_mode_;
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
    return lock_delay_timer_;
}

int Tetris::GetResetCounter() const
{
    return reset_counter_;
}

void Tetris::start_lock_delay_timer()
{
    if (lock_delay_timer_ == -1)
        lock_delay_timer_ = playing_fps_ / 2;
}

void Tetris::reset_lock_delay_timer()
{
    lock_delay_timer_ = -1;
}

void Tetris::tick_lock_delay_timer()
{
    if (lock_delay_timer_ > 0)
        lock_delay_timer_--;
}

void Tetris::spawn_tetromino()
{
    // Bag
    if (bag_.size() == 7)
        generate_bag();

    const int kind = bag_.front();
    bag_.pop_front();

    // Tetromino
    tetromino_ = Tetromino(kind, Point(4, 19));

    // Timers and counter
    lock_delay_timer_ = -1;
    clearing_timer_ = -1;
    reset_counter_ = 0;

    // Hold
    is_hold_available_ = true;

    if (!tetromino_.CanFit(field_))
        is_game_over_ = true;
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
