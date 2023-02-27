#include "tetris.h"
#include <vector>
#include <array>

void test();

int main(int argc, char **argv)
{
    test();
    return 0;
}

enum {
    Empty = 0,
    X,
    Y,
};

using Grid = std::vector<std::array<int,10>>;
using Cell4 = std::array<Point,4>;

void find_piece(const Cell4 &cells, int &kind, int &rotation, Point &pos)
{
    Point min = {999, 999}, max = {-999, -999};
    for (auto pos: cells) {
        min.x = std::min(min.x, pos.x);
        min.y = std::min(min.y, pos.y);
        max.x = std::max(max.x, pos.x);
        max.y = std::max(max.y, pos.y);
    }

    const Point BOXSIZE = max - min + Point(1, 1);
    const Point p0 = min, p1 = max;
    std::swap(min.y, max.y);
    const Point p2 = min, p3 = max;

    const bool corner0 = std::find(cells.begin(), cells.end(), p0) != cells.end();
    const bool corner1 = std::find(cells.begin(), cells.end(), p1) != cells.end();
    const bool corner2 = std::find(cells.begin(), cells.end(), p2) != cells.end();
    const bool corner3 = std::find(cells.begin(), cells.end(), p3) != cells.end();

    // I
    if (BOXSIZE.x == 4) {
        kind = I;
        rotation = 0;
        pos = p0 + Point(1, 0);
    }
    else if (BOXSIZE.y == 4) {
        kind = I;
        rotation = 1;
        pos = p0 + Point(0, 2);
    }
    // O
    else if (BOXSIZE.x == 2 && BOXSIZE.y == 2) {
        kind = O;
        rotation = 0;
        pos = p0;
    }
    else if (BOXSIZE.x == 3 && BOXSIZE.y == 2) {
        // Z
        if (corner0 == 0 &&
            corner1 == 0 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = Z;
            rotation = 0;
            pos = p0 + Point(1, 0);
        }
        else
        // S
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 0 &&
            corner3 == 0) {
            kind = S;
            rotation = 0;
            pos = p0 + Point(1, 0);
        }
        else
        // L
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 0 &&
            corner3 == 1) {
            kind = L;
            rotation = 0;
            pos = p0 + Point(1, 0);
        }
        else
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 1 &&
            corner3 == 0) {
            kind = L;
            rotation = 2;
            pos = p0 + Point(1, 1);
        }
        else
        // J
        if (corner0 == 1 &&
            corner1 == 0 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = J;
            rotation = 0;
            pos = p0 + Point(1, 0);
        }
        else
        if (corner0 == 0 &&
            corner1 == 1 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = J;
            rotation = 2;
            pos = p0 + Point(1, 1);
        }
        else
        // T
        if (corner0 == 1 &&
            corner1 == 0 &&
            corner2 == 0 &&
            corner3 == 1) {
            kind = T;
            rotation = 0;
            pos = p0 + Point(1, 0);
        }
        else
        if (corner0 == 0 &&
            corner1 == 1 &&
            corner2 == 1 &&
            corner3 == 0) {
            kind = T;
            rotation = 2;
            pos = p0 + Point(1, 1);
        }
    }
    else if (BOXSIZE.x == 2 && BOXSIZE.y == 3) {
        // Z
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 0 &&
            corner3 == 0) {
            kind = Z;
            rotation = 1;
            pos = p0 + Point(0, 1);
        }
        else
        // S
        if (corner0 == 0 &&
            corner1 == 0 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = S;
            rotation = 1;
            pos = p0 + Point(0, 1);
        }
        else
        // J
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 1 &&
            corner3 == 0) {
            kind = J;
            rotation = 1;
            pos = p0 + Point(0, 1);
        }
        else
        if (corner0 == 1 &&
            corner1 == 1 &&
            corner2 == 0 &&
            corner3 == 1) {
            kind = J;
            rotation = 3;
            pos = p0 + Point(1, 1);
        }
        else
        // L
        if (corner0 == 1 &&
            corner1 == 0 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = L;
            rotation = 1;
            pos = p0 + Point(0, 1);
        }
        else
        if (corner0 == 0 &&
            corner1 == 1 &&
            corner2 == 1 &&
            corner3 == 1) {
            kind = L;
            rotation = 3;
            pos = p0 + Point(1, 1);
        }
        else
        // T
        if (corner0 == 1 &&
            corner1 == 0 &&
            corner2 == 1 &&
            corner3 == 0) {
            kind = T;
            rotation = 1;
            pos = p0 + Point(0, 1);
        }
        else
        if (corner0 == 0 &&
            corner1 == 1 &&
            corner2 == 0 &&
            corner3 == 1) {
            kind = T;
            rotation = 3;
            pos = p0 + Point(1, 1);
        }
    }
}

void setup_field(Tetris &tetris, const Grid &grid)
{
    std::array<Point,4> cells;
    auto cell = cells.begin();
    const int HEIGHT = grid.size();

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 10; x++) {
            const int kind = grid[y][x];
            const Point pos = {x, HEIGHT - y - 1};

            if (kind == X) {
                assert(cell != cells.end());
                *cell++ = pos;
            }
            else if (kind == Y) {
                tetris.SetFieldCellKind(pos, I);
            }
        }
    }

    int kind = 0;
    int rotation = 0;
    Point pos = {};

    find_piece(cells, kind, rotation, pos);

    tetris.SetTetrominoKind(kind);
    tetris.SetTetrominoRotation(rotation);
    tetris.SetTetrominoPos(pos);
}

void update_frame_ntimes(Tetris &tetris, int operation, int times)
{
    for (int i = 0; i < times; i++)
        tetris.UpdateFrame(operation);
}

void AssertEq(int expected, int actual, int line)
{
    if (expected != actual) {
        printf("\033[0;31mNG\033[0;39m\n");
        printf("error:%d: expected: %d actual: %d\n", line, expected, actual);
        exit(1);
    }
}

void AssertEq(Point expected, Point actual, int line)
{
    if (expected != actual) {
        printf("\033[0;31mNG\033[0;39m\n");
        printf("error:%d: expected: (%d, %d) actual: (%d, %d)\n",
                line, expected.x, expected.y, actual.x, actual.y);
        exit(1);
    }
}

#define ASSERT_EQ(expected,actual) AssertEq((expected),(actual),__LINE__)

void test()
{
    {
        const Grid grid = {
            {0,0,0,0,X,X,X,X,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(I, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(5, 2), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(I, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(7, 3), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,X,X,0,0},
            {0,0,0,0,0,0,X,X,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(O, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(6, 0), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,X,X,0,0,0,0,0},
            {0,0,0,0,X,X,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(Z, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(4, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,X,0,0,0,0,0,0},
            {0,0,X,X,0,0,0,0,0,0},
            {0,0,X,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(Z, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(2, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,X,X,0,0,0,0,0},
            {0,0,X,X,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(S, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(3, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {X,0,0,0,0,0,0,0,0,0},
            {X,X,0,0,0,0,0,0,0,0},
            {0,X,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(S, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(0, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,X,X,X,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(L, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(6, 0), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,X,0,0},
            {0,0,0,0,0,0,0,X,X,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(L, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(7, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {X,X,X,0,0,0,0,0,0,0},
            {X,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(L, tetris.GetTetrominoKind());
        ASSERT_EQ(2, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(1, 2), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,X,X,0,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(L, tetris.GetTetrominoKind());
        ASSERT_EQ(3, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(5, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,X,0,0,0,0,0,0,0},
            {0,0,X,X,X,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(J, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(3, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,X,X,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(J, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(5, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,X,X,X,0},
            {0,0,0,0,0,0,0,0,X,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(J, tetris.GetTetrominoKind());
        ASSERT_EQ(2, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(7, 2), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,X},
            {0,0,0,0,0,0,0,0,0,X},
            {0,0,0,0,0,0,0,0,X,X},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(J, tetris.GetTetrominoKind());
        ASSERT_EQ(3, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(9, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,X,0,0,0},
            {0,0,0,0,0,X,X,X,0,0},
            {0,0,0,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(T, tetris.GetTetrominoKind());
        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(6, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,X,0},
            {0,0,0,0,0,0,0,0,X,X},
            {0,0,0,0,0,0,0,0,X,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(T, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(8, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,0},
            {0,X,X,X,0,0,0,0,0,0},
            {0,0,X,0,0,0,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(T, tetris.GetTetrominoKind());
        ASSERT_EQ(2, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(2, 1), tetris.GetTetrominoPos());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,X,0,0,0,0},
            {0,0,0,0,X,X,0,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(T, tetris.GetTetrominoKind());
        ASSERT_EQ(3, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(5, 1), tetris.GetTetrominoPos());
    }
    // Single ===========================================
    {
        const Grid grid = {
            {0,0,0,0,X,X,X,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {Y,Y,Y,Y,Y,0,Y,Y,Y,Y},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(1, tetris.GetClearedLineCount());
        ASSERT_EQ(100, tetris.GetClearPoints());
    }
    // Double ===========================================
    {
        const Grid grid = {
            {0,0,0,0,0,X,X,0,0,0},
            {0,0,0,0,0,X,0,0,0,0},
            {Y,Y,Y,Y,Y,X,Y,Y,Y,Y},
            {Y,Y,Y,Y,Y,0,Y,Y,Y,Y},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        ASSERT_EQ(J, tetris.GetTetrominoKind());
        ASSERT_EQ(1, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(5, 2), tetris.GetTetrominoPos());

        tetris.UpdateFrame(MOV_DOWN);

        ASSERT_EQ(Point(5, 1), tetris.GetTetrominoPos());
        ASSERT_EQ(29, tetris.GetLockDelayTimer());

        update_frame_ntimes(tetris, 0, 30);

        ASSERT_EQ(2, tetris.GetClearedLineCount());
        ASSERT_EQ(300, tetris.GetClearPoints());
    }
}
