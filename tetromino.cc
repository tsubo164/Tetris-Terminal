#include "tetromino.h"
#include "piece.h"
#include "field.h"

Tetromino::Tetromino()
{
}

Tetromino::Tetromino(int Kind, Point Pos)
    : kind(Kind), pos(Pos)
{
}

Tetromino::~Tetromino()
{
}

bool Tetromino::CanFit(const Field &field) const
{
    for (int i = 0; i < 4; i++) {
        const Point local = GetPiece(kind, rotation).cells[i];
        const Point world = pos + local;
        const int field_cell = field.GetFieldCellKind(world);

        if (field_cell)
            return false;
    }

    return true;
}
