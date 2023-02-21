#include "piece.h"

static char piece_data[9][4][4] =
{
    { // E
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // I
        {0, 0, 0, 0},
        {I, I, I, I},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // O
        {0, O, O, 0},
        {0, O, O, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // S
        {0, S, S, 0},
        {S, S, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // Z
        {Z, Z, 0, 0},
        {0, Z, Z, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // J
        {J, 0, 0, 0},
        {J, J, J, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // L
        {0, 0, L, 0},
        {L, L, L, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // T
        {0, T, 0, 0},
        {T, T, T, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // T corners
        {T, 0, T, 0},
        {0, 0, 0, 0},
        {T, 0, T, 0},
        {0, 0, 0, 0},
    },
};

static Piece piece_states[9][4] = {};

static Point rotate(Point point, int rotation)
{
    if (rotation < 1 || rotation > 4)
        return point;

    Point result = point;

    for (int i = 0; i < rotation; i++) {
        const Point tmp = {result.y, -result.x};
        result = tmp;
    }

    return result;
}

static void init_piece(int kind, int rotation)
{
    Piece &piece = piece_states[kind][rotation];
    piece.kind = kind;
    int cell_index = 0;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            const char cell = piece_data[kind][y][x];

            if (cell) {
                // local := grid(x, invert(y)) - center
                Point local = Point(x, 4 - y - 1) - Point(1, 2);
                local = rotate(local, rotation);

                piece.cells[cell_index++] = local;
            }

            if (cell_index == 4)
                return;
        }
    }
};

void InitializePieces()
{
    // loop over all tetrominoes
    for (int kind = E; kind < T_CORNERS; kind++)
        // loop over 4 rotations
        for (int rot = 0; rot < 4; rot++)
            init_piece(kind, rot);

    // T corners
    for (int rot = 0; rot < 4; rot++)
        init_piece(T_CORNERS, rot);
}

Piece GetPiece(int kind, int rotation)
{
    assert(IsValidCell(kind));
    assert(rotation >= 0 && rotation < 4);

    return piece_states[kind][rotation];
}

Piece GetTcorners(int rotation)
{
    assert(rotation >= 0 && rotation < 4);

    return piece_states[T_CORNERS][rotation];
}

bool IsEmptyCell(int kind)
{
    return kind == E;
}

bool IsSolidCell(int kind)
{
    return kind > E && kind < T_CORNERS;
}

bool IsValidCell(int kind)
{
    return IsEmptyCell(kind) || IsSolidCell(kind);
}
