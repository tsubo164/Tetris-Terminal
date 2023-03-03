#ifndef PIECE_H
#define PIECE_H

#include "point.h"
#include <array>

enum TileKind {
    B = -1, // border
    E = 0, // empty
    I, O, S, Z, J, L, T,
    T_CORNERS,
};

struct Piece {
    std::array<Point, 4> tiles;
    int kind = E;
};

// Tile kind
bool IsEmptyTile(int kind);
bool IsSolidTile(int kind);
bool IsValidTile(int kind);

// Piece
void InitializePieces();
Piece GetPiece(int kind, int rotation);
Piece GetTcorners(int rotation);

#endif
