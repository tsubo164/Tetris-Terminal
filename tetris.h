#ifndef TETRIS_H
#define TETRIS_H

#include "tetromino.h"
#include "scorer.h"
#include "point.h"
#include "piece.h"
#include "field.h"
#include <deque>

enum TetrominoMove {
    MOV_RIGHT     = 1 << 0,
    MOV_LEFT      = 1 << 1,
    MOV_UP        = 1 << 2,
    MOV_DOWN      = 1 << 3,
    MOV_HARDDROP  = 1 << 4,
    ROT_RIGHT     = 1 << 5,
    ROT_LEFT      = 1 << 6,
    HOLD_PIECE    = 1 << 7,
};

class Tetris {
public:
    Tetris();
    ~Tetris();

    // Game controls
    void PlayGame();
    void QuitGame();
    void PauseGame();
    bool IsPlaying() const;
    bool IsGameOver() const;
    bool IsPaused() const;

    // Options
    void SetPreviewCount(int count);
    void SetGhostEnable(bool enable);
    void SetHoldEnable(bool enable);
    bool IsGhostEnable() const;
    bool IsHoldEnable() const;

    // Tick Game
    void UpdateFrame(int move);

    // Field
    int GetFieldTileKind(Point pos) const;
    int GetClearedLineCount() const;
    void GetClearedLines(int *cleared_line_y) const;
    bool IsPerfectClear() const;

    // Piece
    int GetPieceKindList(int index) const;
    Piece GetCurrentPiece() const;
    Piece GetGhostPiece() const;
    Piece GetHoldPiece() const;
    Piece GetNextPiece(int index) const;
    int GetNextPieceCount() const;
    bool IsHoldAvailable() const;

    // Info
    int GetLevel() const;
    int GetScore() const;
    int GetTotalLineCount() const;
    int GetComboCounter() const;
    int GetComboPoints() const;
    int GetTspinKind() const;
    int GetClearPoints() const;
    int GetBackToBackCounter() const;

    // Debug
    void SetDebugMode();
    bool IsDebugMode() const;
    void SetTetrominoKind(int kind);
    int GetTetrominoKind() const;
    void SetTetrominoRotation(int rotation);
    int GetTetrominoRotation() const;
    void SetTetrominoPos(Point pos);
    Point GetTetrominoPos() const;
    void SetFieldTileKind(Point pos, int kind);
    int GetLockDelayTimer() const;
    int GetResetCounter() const;

    float GetGravity() const;
    void SetGravity(float gravity);
    void SetGravityDrop(float gravity_drop);

    void EnableLog(bool enable);

private:
    Tetromino tetromino_;
    Tetromino ghost_;
    Tetromino hold_;
    Field field_;

    Scorer scorer_;
    std::deque<int> bag_;

    bool is_playing_ = false;
    bool is_game_over_ = false;
    bool is_paused_ = false;
    bool debug_mode_ = false;
    int playing_fps_ = 60;

    int preview_count_ = 3;
    bool is_hold_available_ = false;
    bool is_hold_enable_ = true;
    bool is_ghost_enable_ = true;

    unsigned long frame_ = 0;

    int lock_delay_timer_ = -1;
    int reset_counter_ = -1;
    bool need_spawn_ = false;

    float gravity_drop_ = 0;
    float gravity_ = 1./60;
    int last_move_ = 0;
    Point last_kick_ = {};
    int tspin_kind_ = 0;

    bool hard_drop(Tetromino &tet);
    bool drop_piece(int move);
    bool shift_piece(int move);
    bool rotate_piece(int move);
    bool move_piece(int move);
    bool has_piece_landed() const;
    void hold_piece();
    void update_ghost();
    int detect_tspin() const;

    void start_lock_delay_timer();
    void reset_lock_delay_timer();
    void tick_lock_delay_timer();
    void reset_all_timers();

    bool spawn_tetromino();
    void generate_bag();

    void add_log(int move);
};

#endif
