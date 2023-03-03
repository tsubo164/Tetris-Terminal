#include "tetris.h"
#include <vector>
#include <array>

void test();

int main(int argc, char **argv)
{
    test();
    return 0;
}

using Grid = std::vector<std::array<int,10>>;

static Point bbox_min(const Piece &piece)
{
    Point min = {9999, 9999};
    for (auto pos: piece.cells) {
        min.x = std::min(min.x, pos.x);
        min.y = std::min(min.y, pos.y);
    }
    return min;
}

void find_piece_state(const Piece &piece, int &kind, int &rotation, Point &pos)
{
    const Point piece_min = bbox_min(piece);

    for (int k = E + 1; k < T_CORNERS; k++) {
        for (int r = 0; r < 4; r++) {
            const Piece pattern = GetPiece(k, r);
            const Point pattern_min = bbox_min(pattern);
            bool match = true;

            for (auto piece_pos: piece.cells) {
                match &= std::any_of(
                        pattern.cells.begin(),
                        pattern.cells.end(),
                        [=](Point pattern_pos){
                            return piece_pos - piece_min == pattern_pos - pattern_min;
                        });
            }

            if (match) {
                kind = k;
                rotation = r;
                pos = piece_min - pattern_min;
                return;
            }
        }
    }

    printf("kind: %d, rotation: %d\n", kind, rotation);
    assert(!"should not reach here");
}

void setup_field(Tetris &tetris, const Grid &grid)
{
    Piece piece;
    auto cell = piece.cells.begin();
    const int HEIGHT = grid.size();

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 10; x++) {
            const int kind = grid[y][x];
            const Point pos = {x, HEIGHT - y - 1};

            if (kind == I) {
                // Active piece symbol in grid
                assert(cell != piece.cells.end());
                *cell++ = pos;
            }
            else if (kind == O) {
                // Static piece symbol in grid
                tetris.SetFieldCellKind(pos, I);
            }
        }
    }

    int kind = 0;
    int rotation = 0;
    Point pos = {};

    find_piece_state(piece, kind, rotation, pos);

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

void AssertNe(Point expected, Point actual, int line)
{
    if (expected == actual) {
        printf("\033[0;31mNG\033[0;39m\n");
        printf("error:%d: expected: (%d, %d) actual: (%d, %d)\n",
                line, expected.x, expected.y, actual.x, actual.y);
        exit(1);
    }
}

#define ASSERT_EQ(expected,actual) AssertEq((expected),(actual),__LINE__)
#define ASSERT_NE(expected,actual) AssertNe((expected),(actual),__LINE__)

void test()
{
    {
        const Grid grid = {
            {0,0,0,0,I,I,I,I,0,0},
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
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,0,0,I,0,0},
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
            {0,0,0,0,0,0,I,I,0,0},
            {0,0,0,0,0,0,I,I,0,0},
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
            {0,0,0,I,I,0,0,0,0,0},
            {0,0,0,0,I,I,0,0,0,0},
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
            {0,0,0,I,0,0,0,0,0,0},
            {0,0,I,I,0,0,0,0,0,0},
            {0,0,I,0,0,0,0,0,0,0},
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
            {0,0,0,I,I,0,0,0,0,0},
            {0,0,I,I,0,0,0,0,0,0},
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
            {I,0,0,0,0,0,0,0,0,0},
            {I,I,0,0,0,0,0,0,0,0},
            {0,I,0,0,0,0,0,0,0,0},
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
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,I,I,I,0,0},
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
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,0,0,I,0,0},
            {0,0,0,0,0,0,0,I,I,0},
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
            {I,I,I,0,0,0,0,0,0,0},
            {I,0,0,0,0,0,0,0,0,0},
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
            {0,0,0,0,I,I,0,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
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
            {0,0,I,0,0,0,0,0,0,0},
            {0,0,I,I,I,0,0,0,0,0},
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
            {0,0,0,0,0,I,I,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
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
            {0,0,0,0,0,0,I,I,I,0},
            {0,0,0,0,0,0,0,0,I,0},
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
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,0,0,0,0,I,I},
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
            {0,0,0,0,0,0,I,0,0,0},
            {0,0,0,0,0,I,I,I,0,0},
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
            {0,0,0,0,0,0,0,0,I,0},
            {0,0,0,0,0,0,0,0,I,I},
            {0,0,0,0,0,0,0,0,I,0},
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
            {0,I,I,I,0,0,0,0,0,0},
            {0,0,I,0,0,0,0,0,0,0},
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
            {0,0,0,0,0,I,0,0,0,0},
            {0,0,0,0,I,I,0,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
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
            {0,0,0,0,I,I,I,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {O,O,O,O,O,0,O,O,O,O},
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
            {0,0,0,0,0,I,I,0,0,0},
            {0,0,0,0,0,I,0,0,0,0},
            {O,O,O,O,O,I,O,O,O,O},
            {O,O,O,O,O,0,O,O,O,O},
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
    // Triple ===========================================
    {
        const Grid grid = {
            {0,0,I,I,0,0,0,0,0,0},
            {0,0,0,I,0,0,0,0,0,0},
            {0,0,0,I,0,0,0,O,O,0},
            {O,O,0,0,O,O,O,O,O,O},
            {O,O,O,0,O,O,O,O,O,O},
            {O,O,O,0,O,O,O,O,O,O},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(3, tetris.GetClearedLineCount());
        ASSERT_EQ(500, tetris.GetClearPoints());
    }
    // Tetris ===========================================
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,O,O,0,0,0,I},
            {O,O,O,O,O,O,O,O,O,I},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(4, tetris.GetClearedLineCount());
        ASSERT_EQ(800, tetris.GetClearPoints());
    }
    // Perfect clear ===========================================
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,I,I},
            {0,0,0,0,0,0,0,0,I,I},
            {O,O,O,O,O,O,O,O,0,0},
            {O,O,O,O,O,O,O,O,0,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(1, tetris.IsPerfectClear());
        ASSERT_EQ(2, tetris.GetClearedLineCount());
        ASSERT_EQ(1200, tetris.GetClearPoints());
    }
    {
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,0,0,0,0,0,I},
            {0,0,0,0,0,0,0,0,0,I},
            {O,O,O,O,O,O,O,O,O,I},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
        };

        Tetris tetris;
        tetris.SetDebugMode();
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);

        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(4, tetris.GetClearedLineCount());
        ASSERT_EQ(800, tetris.GetClearPoints());
        tetris.UpdateFrame(0);

        tetris.SetTetrominoKind(1);
        tetris.SetTetrominoRotation(1);
        tetris.SetTetrominoPos(Point(9, 10));
        tetris.UpdateFrame(MOV_HARDDROP);

        ASSERT_EQ(1, tetris.IsPerfectClear());
        ASSERT_EQ(4, tetris.GetClearedLineCount());
        ASSERT_EQ(3200, tetris.GetClearPoints());
    }
    {
        // Fixing failure to set piece
        // crash log: tests/log_tetris_1084797668.txt
        const Grid grid = {
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0},
            {O,O,O,0,0,0,0,0,0,0},
            {O,O,O,O,O,0,0,0,I,I},
            {O,O,O,O,O,O,O,O,0,I},
            {O,O,O,O,O,O,0,O,O,I},
            {O,O,O,O,O,O,O,O,0,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
            {O,O,O,O,O,O,O,O,O,0},
        };

        Tetris tetris;
        tetris.PlayGame();
        tetris.UpdateFrame(0);

        setup_field(tetris, grid);
        ASSERT_EQ(6, tetris.GetTetrominoKind());
        ASSERT_EQ(3, tetris.GetTetrominoRotation());
        ASSERT_EQ(Point(9, 6), tetris.GetTetrominoPos());

        tetris.SetGravityDrop(-0.8186);
        tetris.SetGravity(0.2598);
        tetris.UpdateFrame(ROT_RIGHT);

        ASSERT_EQ(0, tetris.GetTetrominoRotation());
        ASSERT_NE(Point(9, 5), tetris.GetTetrominoPos());

        update_frame_ntimes(tetris, 0, 30);
    }
}
