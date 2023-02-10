#ifndef TETRIS_H
#define TETRIS_H

#include "point.h"
#include "field.h"
#include "cell.h"

enum TetrominoAction {
    MOV_RIGHT = 1 << 0,
    MOV_LEFT  = 1 << 1,
    MOV_UP    = 1 << 2,
    MOV_DOWN  = 1 << 3,
    ROT_RIGHT = 1 << 4,
    ROT_LEFT  = 1 << 5,
};

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

#endif
