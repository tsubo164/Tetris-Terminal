#ifndef TETRIS_H
#define TETRIS_H

#include "point.h"
#include "piece.h"
#include "field.h"
#include "cell.h"

struct Tetromino {
    int kind;
    int rotation;
    Point pos;
};

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
    bool IsPlaying();

    void MoveTetromino(int action);
    void UpdateFrame();

    Cell GetTetrominoCell(int index);

    int GetClearingTimer();

    void SetDebugMode();
    bool IsDebugMode();
    void ChangeTetrominoKind(int kind);

private:
    Tetromino tetromino;

    bool is_playing = false;
    bool debug_mode = false;
    int playing_fps = 60;

    unsigned long frame = 0;
    int period = 60;
    int lock_down_counter = -1;
    int clearing_timer = -1;

    void reset_lock_down_counter();
    bool can_fit(const Tetromino &tet);
    bool kick_wall(Tetromino &tet, int old_rotation);
    void spawn_tetromino();
};

#endif
