#ifndef PIECE_H
#define PIECE_H

#include "point.h"
#include "cell.h"
#include <array>

struct Piece {
    std::array<Point, 4> cells;
    int kind = E;
};

void InitializePieces();

Piece GetPiece(int kind, int rotation);

#endif
