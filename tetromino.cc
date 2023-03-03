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
    const Piece piece = GetPiece(kind, rotation);

    for (auto cell: piece.tiles) {
        const Point world = pos + cell;
        const int kind = field.GetCellKind(world);

        if (!IsEmptyCell(kind))
            return false;
    }

    return true;
}

// TTC's super rotation system.
// https://tetris.wiki/Super_Rotation_System
static const Point offset_table_jlstz [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {+1, 0}, {+1,-1}, { 0,+2}, {+1,+2}},
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0, 0}, {-1, 0}, {-1,-1}, { 0,+2}, {-1,+2}},
};
static const Point offset_table_i [4][5] = {
    {{ 0, 0}, {-1, 0}, {+2, 0}, {-1, 0}, {+2, 0}},
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0,+1}, { 0,-2}},
    {{-1,+1}, {+1,+1}, {-2,+1}, {+1, 0}, {-2, 0}},
    {{ 0,+1}, { 0,+1}, { 0,+1}, { 0,-1}, { 0,+2}},
};
static const Point offset_table_o [4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{ 0,-1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{-1,-1}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}},
};

bool Tetromino::KickWall(const Field &field, int old_rotation)
{
    const int new_rotation = rotation;
    const Point *offsets0 = nullptr;
    const Point *offsets1 = nullptr;

    // Pick up offset data based on tetromino_ kind.
    if (kind == I) {
        offsets0 = offset_table_i[old_rotation];
        offsets1 = offset_table_i[new_rotation];
    }
    else if (kind == O) {
        offsets0 = offset_table_o[old_rotation];
        offsets1 = offset_table_o[new_rotation];
    }
    else {
        offsets0 = offset_table_jlstz[old_rotation];
        offsets1 = offset_table_jlstz[new_rotation];
    }

    // Tests against 5 offsets.
    for (int i = 0; i < 5; i++) {
        const Point offset0 = offsets0[i];
        const Point offset1 = offsets1[i];

        Tetromino test = *this;
        test.pos += offset0 - offset1;

        if (test.CanFit(field)) {
            pos = test.pos;
            return true;
        }
    }
    return false;
}
