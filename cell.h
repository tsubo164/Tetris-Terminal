#ifndef CELL_H
#define CELL_H

enum CellKind {
    B = -1, // border
    E = 0, // empty
    I, O, S, Z, J, L, T,
    CELL_END,
};

struct Cell {
    int kind = 0;
    Point pos;
};

inline bool IsEmptyCell(int kind)
{
    return kind == E;
}

inline bool IsSolidCell(int kind)
{
    return kind > E && kind < CELL_END;
}

inline bool IsValidCell(int kind)
{
    return IsEmptyCell(kind) || IsSolidCell(kind);
}

#endif
