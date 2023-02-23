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

static const Point SPAWN_POS = {4, 19};

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

    // Score
    scorer_.Reset();

    // Start
    ghost_ = Tetromino();
    hold_ = Tetromino();

    need_spawn_ = true;
}

void Tetris::start_lock_delay_timer()
{
    if (lock_delay_timer_ == -1)
        lock_delay_timer_ = playing_fps_ / 2;
}

void Tetris::reset_lock_delay_timer()
{
    if (lock_delay_timer_ > 0 && reset_counter_ < 16) {
        lock_delay_timer_ = -1;
        reset_counter_++;
    }
}

void Tetris::tick_lock_delay_timer()
{
    if (lock_delay_timer_ > 0)
        lock_delay_timer_--;
}

void Tetris::reset_all_timers()
{
    lock_delay_timer_ = -1;
    reset_counter_ = 0;
}

void Tetris::spawn_tetromino()
{
    // Bag
    if (bag_.size() == 7)
        generate_bag();

    const int kind = bag_.front();
    bag_.pop_front();

    // Tetromino
    tetromino_ = Tetromino(kind, SPAWN_POS);

    // Timers and counter
    reset_all_timers();

    // Hold
    is_hold_available_ = true;

    // Score
    scorer_.Start();

    if (!tetromino_.CanFit(field_))
        is_game_over_ = true;

    need_spawn_ = false;
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
    if (!IsDebugMode())
        gravity_drop_ -= gravity_;

    // Move
    if (action & MOV_LEFT)
        moved.pos.x--;

    if (action & MOV_RIGHT)
        moved.pos.x++;

    if ((action & MOV_UP) && IsDebugMode())
        moved.pos.y++;

    if (action & MOV_DOWN)
        gravity_drop_ -= 60./60;

    while (gravity_drop_ < -1) {
        moved.pos.y--;
        gravity_drop_ += 1;
        if (!moved.CanFit(field_)) {
            moved.pos.y++;
            break;
        }
    }

    if (moved.pos != current.pos)
        has_moved = moved.CanFit(field_);

    // Rot
    if (action & ROT_LEFT)
        moved.rotation = (current.rotation - 1 + 4) % 4;

    if (action & ROT_RIGHT)
        moved.rotation = (current.rotation + 1) % 4;

    if (moved.rotation != current.rotation)
        has_moved = has_moved || moved.KickWall(field_, current.rotation);

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

void Tetris::hold_piece()
{
    if (!IsHoldEnable())
        return;

    if (IsEmptyCell(hold_.kind)) {
        hold_ = Tetromino(tetromino_.kind, Point());
        need_spawn_ = true;
    }
    else if (IsHoldAvailable()) {
        std::swap(tetromino_.kind, hold_.kind);
        tetromino_ = Tetromino(tetromino_.kind, SPAWN_POS);
        hold_ = Tetromino(hold_.kind, Point());
        reset_all_timers();
    }

    is_hold_available_ = false;
}

void Tetris::update_ghost()
{
    if (!IsGhostEnable()) {
        ghost_.kind = E;
        return;
    }

    ghost_ = tetromino_;
    drop_piece(ghost_);
    if (ghost_.pos == tetromino_.pos)
        ghost_.kind = E;
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
        scorer_.Commit();
        gravity_ = get_gravity(GetLevel());

        field_.ClearLines();
        need_spawn_ = true;
        clearing_timer_ = -1;
        return;
    }

    // Spawn
    if (need_spawn_)
        spawn_tetromino();

    // Actions
    if (action & HOLD_PIECE) {
        hold_piece();
        return;
    }
    else if (action & MOV_HARDDROP) {
        const int old_y = tetromino_.pos.y;
        drop_piece(tetromino_);
        lock_delay_timer_ = 0;

        scorer_.AddHardDrop(old_y - tetromino_.pos.y);
    }
    else {
        const bool moved = move_piece(action);
        if (moved)
            reset_lock_delay_timer();

        if (moved && (action & MOV_DOWN))
            scorer_.AddSoftDrop();
    }
    if (action)
        last_action_ = action;

    const bool landed = has_landed();
    if (landed) {
        gravity_drop_ = 0.;
        start_lock_delay_timer();
    }

    // Ghost
    update_ghost();

    // Locking
    if (lock_delay_timer_ == 0 && landed) {
        field_.SetPiece(GetCurrentPiece());

        const int cleared_lines = GetClearedLineCount();

        // T-Spin
        if (tetromino_.kind == T && (last_action_ & (ROT_LEFT | ROT_RIGHT)))
            scorer_.AddTspin(tetromino_.pos, tetromino_.rotation, field_);

        if (cleared_lines > 0) {
            scorer_.AddLineClear(cleared_lines);
            // start clear lines
            tetromino_.kind = E;
            clearing_timer_ = 20;
        }
        else {
            // spawn
            need_spawn_ = true;
        }
    }
    else {
        tick_lock_delay_timer();
    }

    frame_++;
}

int Tetris::GetFieldCellKind(Point pos) const
{
    return field_.GetCellKind(pos);
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

int Tetris::GetNextPieceCount() const
{
    return preview_count_;
}

bool Tetris::IsHoldAvailable() const
{
    return is_hold_available_;
}

int Tetris::GetLevel() const
{
    return scorer_.GetLevel();
}

int Tetris::GetScore() const
{
    return scorer_.GetScore();
}

int Tetris::GetTotalLineCount() const
{
    return scorer_.GetLines();
}

int Tetris::GetComboCounter() const
{
    return scorer_.GetComboCounter();
}

int Tetris::GetComboPoints() const
{
    return scorer_.GetComboPoints();
}

int Tetris::GetTspinKind() const
{
    return scorer_.GetTspinKind();
}

int Tetris::GetTspinPoints() const
{
    return scorer_.GetTspinPoints();
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

float Tetris::GetGravity() const
{
    return gravity_;
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

void Tetris::SetGhostEnable(bool enable)
{
    is_ghost_enable_ = enable;
}

void Tetris::SetHoldEnable(bool enable)
{
    is_hold_enable_ = enable;
}

bool Tetris::IsGhostEnable() const
{
    return is_ghost_enable_;
}

bool Tetris::IsHoldEnable() const
{
    return is_hold_enable_;
}
