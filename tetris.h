#ifndef TETRIS_H
#define TETRIS_H

#include "point.h"
#include "piece.h"
#include "field.h"
#include <deque>

enum TetrominoAction {
    MOV_RIGHT = 1 << 0,
    MOV_LEFT  = 1 << 1,
    MOV_UP    = 1 << 2,
    MOV_DOWN  = 1 << 3,
    ROT_RIGHT = 1 << 4,
    ROT_LEFT  = 1 << 5,
};

class Tetris {
public:
    Tetris();
    ~Tetris();

    void PlayGame();
    void QuitGame();
    bool IsPlaying() const;

    // Move
    void MoveTetromino(int action);
    void UpdateFrame();
    void UpdateFrame(int action);

    // Field
    int GetFieldCellKind(Point pos) const;
    int GetClearedLineCount() const;
    void GetClearedLines(int *cleared_line_y) const;
    int GetClearingTimer() const;

    // Piece
    int GetPieceKindList(int index) const;
    Piece GetCurrentPiece() const;
    Piece GetNextPiece(int index) const;

    // Debug
    void SetDebugMode();
    bool IsDebugMode() const;
    void ChangeTetrominoKind(int kind);
    int GetLockDelayTimer() const;

private:
    struct Tetromino {
        int kind;
        int rotation;
        Point pos;
    };

    Tetromino tetromino_;
    Field field_;

    std::deque<int> bag_;

    bool is_playing = false;
    bool debug_mode = false;
    int playing_fps = 60;

    unsigned long frame = 0;
    int period = 60;
    int lock_delay_timer = -1;
    int clearing_timer = -1;

    float drop_ = 0;
    float gravity_ = 1./60;

    bool move_piece(int action);
    bool has_landed();
    void start_lock_delay_timer();
    void reset_lock_delay_timer();
    void tick_lock_delay_timer();

    bool can_fit(const Tetromino &tet);
    bool kick_wall(Tetromino &tet, int old_rotation);
    void spawn_tetromino();

    void generate_bag();
};

#endif
