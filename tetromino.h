#ifndef TETROMINO_H
#define TETROMINO_H

#include "point.h"
#include "field.h"

class Tetromino {
public:
    Tetromino();
    Tetromino(int kind, Point pos);
    ~Tetromino();

    bool CanFit(const Field &field) const;
    bool KickWall(const Field &field, int old_rotation);

    int kind = E;
    int rotation = 0;
    Point pos = {0, 0};
};

#endif
