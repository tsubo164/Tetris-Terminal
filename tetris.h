#ifndef TETRIS_H
#define TETRIS_H

constexpr int FIELD_WIDTH = 10 + 2;
constexpr int FIELD_HEIGHT = 20 + 2;

enum CellKind {
    E = 0, // empty
    I, O, S, Z, J, L, T,
    B, // border
};

enum TetrominoAction {
    MOV_RIGHT = 1 << 0,
    MOV_LEFT  = 1 << 1,
    MOV_UP    = 1 << 2,
    MOV_DOWN  = 1 << 3,
    ROT_RIGHT = 1 << 4,
    ROT_LEFT  = 1 << 5,
};

struct Point {
    int x = 0, y = 0;
};

struct Cell {
    int kind = 0;
    Point pos;
};

void PlayGame();
void QuitGame();
bool IsPlaying();

void MoveTetromino(int action);
void UpdateFrame();

Cell GetTetrominoCell(int index);
int GetFieldCellKind(int x, int y);

void SetDebugMode();

#endif
