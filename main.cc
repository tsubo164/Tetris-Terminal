#include "tetris.h"

#include <locale.h>
#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static int initialize_screen();
static void finalize_screen();
static int input_key();
static void render();

static const int SCREEN_HEIGHT = FIELD_HEIGHT + 2;

// Counters
static double fps = 0.0;
static unsigned long frame = 0;
static bool is_drawing_ghost = false;
static int preview_count = 1;

Tetris tetris;
Point global_offset = {1, 1};

int main(int argc, char **argv)
{
    // Arguments
    if (argc == 2) {
        if (!strcmp(argv[1], "-d")) {
            tetris.SetDebugMode();
        }
        else {
            fprintf(stderr, "error: unknown option: %s\n", argv[1]);
            return 1;
        }
    }

    // Screen
    if (initialize_screen())
        return 1;

    // Timer
    auto start = std::chrono::steady_clock::now();

    tetris.PlayGame();

    while (tetris.IsPlaying()) {

        // Input
        const int action = input_key();

        // Game logic
        tetris.UpdateFrame(action);

        // Rednering
        render();

        // Measure frame cost
        const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - start;
        const double elapsed_sec = dur.count();
        const long remaining_millisec = 1000 * (1./60 - elapsed_sec);

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(remaining_millisec));

        // Restart timer
        start = std::chrono::steady_clock::now();
        frame++;

        if (frame % 10 == 0)
            fps = 1. / elapsed_sec;
    }

    // Clean up
    finalize_screen();

    return 0;
}

static void draw_str(int x, int y, const char *str)
{
    const int X = x + global_offset.x;
    const int Y = y + global_offset.y;

    mvprintw(SCREEN_HEIGHT - Y - 1, X, str);
}

static void draw_int(int x, int y, int number)
{
    static char buf[64] = {'\0'};

    sprintf(buf, "%5d", number);
    draw_str(x, y, buf);
}

static void draw_int(int number)
{
    static char buf[64] = {'\0'};

    sprintf(buf, "%3d", number);
    addstr(buf);
}

static void draw_flt(int x, int y, float number)
{
    static char buf[64] = {'\0'};

    sprintf(buf, "%g", number);
    draw_str(x, y, buf);
}

static void draw_square(int x, int y, int kind)
{
    const char *s = ".";

    switch (kind) {
        case E:
            s = ".";
            break;

        case I: case O: case S: case Z: case J: case L: case T:
            s = "\u25A3";
            break;

        case B:
            s = "\u25A2";
            break;

        default:
            s = " ";
            break;
    }

    if (is_drawing_ghost)
        s = "\u25A2";

    draw_str(x, y, s);
}

static const int DEFAULT_COLOR_PAIR = CELL_END;

static void draw_cell(int x, int y, int kind)
{
    if (IsEmptyCell(kind) && !tetris.IsDebugMode())
        return;

    if (IsSolidCell(kind))
        attrset(COLOR_PAIR(kind));
    else
        attrset(COLOR_PAIR(DEFAULT_COLOR_PAIR));

    draw_square(x, y, kind);

    attrset(0);
}

static void draw_tetromino()
{
    const Piece piece = tetris.GetCurrentPiece();

    if (IsEmptyCell(piece.kind))
        return;

    for (auto pos: piece.cells) {
        draw_cell(pos.x, pos.y, piece.kind);
    }
}

static void draw_ghost()
{
    const Piece piece = tetris.GetGhostPiece();

    if (IsEmptyCell(piece.kind))
        return;

    is_drawing_ghost = true;
    for (auto pos: piece.cells) {
        draw_cell(pos.x, pos.y, piece.kind);
    }
    is_drawing_ghost = false;
}

static void draw_borders()
{
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        // Side borders
        draw_cell(-1, y, B);
        draw_cell(FIELD_WIDTH, y, B);
    }
    for (int x = -1; x < FIELD_WIDTH + 1; x++) {
        {
            // Bottom border
            const int y = -1;
            const int kind = tetris.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
        {
            // Top border
            const int y = FIELD_HEIGHT;
            const int kind = tetris.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
    }
}

static void draw_field()
{
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = tetris.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
    }

    const int clearing_timer = tetris.GetClearingTimer();
    if (clearing_timer == -1)
        return;

    const int CLEARED_COUNT = tetris.GetClearedLineCount();
    int cleared_lines[4] = {0};
    tetris.GetClearedLines(cleared_lines);

    const int duration = 20;
    const int frame_per_cell = duration / 5;
    const int erase = clearing_timer / frame_per_cell;

    for (int i = 0; i < CLEARED_COUNT; i++) {
        const int cleared_y = cleared_lines[i];

        for (int x = erase; x < 10 - erase; x++) {
            draw_str(x, cleared_y, " ");
        }
    }
}

static void draw_info()
{
    {
        int x = 19, y = 19;

        draw_str(x, y--, "SCORE");
        draw_int(x, y--, tetris.GetScore());

        y--;
        draw_str(x, y--, "LINES");
        draw_int(x, y--, tetris.GetTotalLineCount());

        y--;
        draw_str(x, y--, "LEVEL");
        draw_int(x, y--, tetris.GetLevel());

        y--;
        draw_str(x, y--, "FPS");
        draw_flt(x, y--, fps);

        y--;
        draw_str(x, y--, "Q: Quit");
        draw_str(x, y--, "R: Reset");
        draw_str(x, y--, "H: Move L");
        draw_str(x, y--, "L: Move R");
        draw_str(x, y--, "J: Sof");
        draw_str(x, y--, "M: Hard");
        draw_str(x, y--, "D: Spin L");
        draw_str(x, y--, "F: Spin R");
        draw_str(x, y--, "ESC: Pause");
    }
    {
        int x = 30, y = 7;
        draw_str(x, y--, "1: Preview #");
        draw_str(x, y--, "2: Ghost");
        draw_str(x, y--, "3: Hold");
    }
    {
        int x = 13, y = 19;

        draw_str(x, y, "NEXT");

        for (int i = 0; i < 14; i++) {
            const Piece next = tetris.GetNextPiece(i);

            if (next.kind == E)
                continue;

            for (const auto &pos: next.cells) {
                draw_cell(x + pos.x + 1, y + pos.y - 2 - i * 3, next.kind);
            }
        }
    }
}

static void draw_debug()
{
    if (!tetris.IsDebugMode())
        return;

    int x = 0, y = -3;
    draw_str(x, y--, "[DEBUG]");
    draw_str(x, y--, "Lock Delay Timer:");
    draw_int(tetris.GetLockDelayTimer());
    draw_str(x, y--, "Reset Counter:   ");
    draw_int(tetris.GetResetCounter());

    draw_str(x, y, "Next Piece Kinds: ");
    for (int i = 0; i < 14; i++) {
        const int kind = tetris.GetPieceKindList(i);
        draw_str(x + 18 + i, y, std::to_string(kind).c_str());
    }
}

static void draw_game_over()
{
    if (!tetris.IsGameOver())
        return;

    draw_str(0, 10, "GAME OVER");
}

static void draw_pause()
{
    if (!tetris.IsPaused())
        return;

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            draw_str(x, y, " ");
        }
    }
    draw_str(2, 10, "PAUSE");
}

void render()
{
    erase();

    draw_borders();
    draw_field();
    draw_ghost();
    draw_tetromino();
    draw_info();
    draw_debug();

    draw_game_over();
    draw_pause();

    refresh();
}

static const int DEFAULT_FG_COLOR = 10;
static const int DEFAULT_BG_COLOR = 11;

static void assign_color(int pair_id, int r, int g, int b)
{
    static int fg_color = DEFAULT_BG_COLOR + 1;

    init_color(fg_color, r, g, b);
    init_pair(pair_id, fg_color, DEFAULT_BG_COLOR);

    fg_color++;
}

static void initialize_colors()
{
    start_color();

    init_color(DEFAULT_BG_COLOR, 160, 160, 160);
    init_color(DEFAULT_FG_COLOR, 1000, 1000, 1000);
    init_pair(DEFAULT_COLOR_PAIR, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
    bkgd(COLOR_PAIR(DEFAULT_COLOR_PAIR));

    assign_color(I, 0, 1000, 1000);
    assign_color(O, 1000, 1000, 0);
    assign_color(S, 0, 1000, 0);
    assign_color(Z, 1000, 0, 0);
    assign_color(J, 100, 300, 1000);
    assign_color(L, 1000, 500, 0);
    assign_color(T, 1000, 0, 500);
    assign_color(DEFAULT_COLOR_PAIR, 800, 800, 800);
}

static int initialize_screen()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    initialize_colors();

    return 0;
}

static void finalize_screen()
{
    endwin();
}

static int input_key()
{
    const int key = getch();
    int action = 0;

    switch (key) {
    case 'd': action = ROT_LEFT; break;
    case 'f': action = ROT_RIGHT; break;
    case 'h': action = MOV_LEFT; break;
    case 'l': action = MOV_RIGHT; break;
    case 'k': action = MOV_UP; break;
    case 'j': action = MOV_DOWN; break;
    case 'm': action = MOV_HARDDROP; break;

    case '1':
        preview_count = preview_count == 6 ? 1 : preview_count + 1;
        tetris.SetPreviewCount(preview_count);
        break;

              /*
    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        tetris.ChangeTetrominoKind(key - '1' + 1);
        break;
              */

    case 'r':
        frame = 0;
        tetris.PlayGame();
        break;

    case 27: // ESC
        tetris.PauseGame();
        break;

    case 'q':
        tetris.QuitGame();
        break;

    default:
        break;
    }

    return action;
}
