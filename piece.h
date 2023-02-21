#ifndef PIECE_H
#define PIECE_H

#include "point.h"
#include <array>

enum CellKind {
    B = -1, // border
    E = 0, // empty
    I, O, S, Z, J, L, T,
    T_CORNERS,
};

struct Piece {
    std::array<Point, 4> cells;
    int kind = E;
};

// Cell kind
bool IsEmptyCell(int kind);
bool IsSolidCell(int kind);
bool IsValidCell(int kind);

// Piece
void InitializePieces();
Piece GetPiece(int kind, int rotation);
Piece GetTcorners(int rotation);

#endif
