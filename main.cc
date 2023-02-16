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

Tetris tetris;


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

static void draw_char(int x, int y, char ch)
{
    mvaddch(SCREEN_HEIGHT - y - 1, x, ch);
}

static void draw_str(int x, int y, const char *str)
{
    mvaddstr(SCREEN_HEIGHT - y - 1, x, str);
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

    mvprintw(SCREEN_HEIGHT - y - 1, x, s);
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
        draw_cell(pos.x + 1, pos.y + 1, piece.kind);
    }
}

static void draw_ghost()
{
    const Piece piece = tetris.GetGhostPiece();

    if (IsEmptyCell(piece.kind))
        return;

    is_drawing_ghost = true;
    for (auto pos: piece.cells) {
        draw_cell(pos.x + 1, pos.y + 1, piece.kind);
    }
    is_drawing_ghost = false;
}

static void draw_field()
{
    // Borders
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        draw_cell(0, y + 1, B);
        draw_cell(FIELD_WIDTH + 1, y + 1, B);
    }
    for (int x = 0; x < FIELD_WIDTH + 2; x++) {
        draw_cell(x, 0, B);
        draw_cell(x, FIELD_HEIGHT + 1, (x < 3 || x > 8) ? B : E);
    }

    // Field
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = tetris.GetFieldCellKind(Point(x, y));
            draw_cell(x + 1, y + 1, kind);
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
            draw_char(x + 1, cleared_y + 1, ' ');
        }
    }
}

static void draw_info()
{
    {
        draw_str(20, 20, "SCORE");
        draw_str(20, 19, std::to_string(tetris.GetScore()).c_str());
    }
    {
        draw_str(20, 17, "LINES");
        draw_str(20, 16, std::to_string(tetris.GetTotalLineCount()).c_str());
    }
    {
        draw_str(20, 14, "LEVEL");
        draw_str(20, 13, std::to_string(tetris.GetLevel()).c_str());
    }

    const std::string fpsstr = "fps: " + std::to_string(fps);
    draw_str(20, 11, fpsstr.c_str());
    draw_str(20, 10, "Press 'Q' to quit");

    const std::string lock_timer = "Lock Delay Timer: " +
        std::to_string(tetris.GetLockDelayTimer());
    draw_str(20, 8, lock_timer.c_str());

    const std::string reset_counter = "Reset Counter:    " +
        std::to_string(tetris.GetResetCounter());
    draw_str(20, 7, reset_counter.c_str());

    for (int i = 0; i < 14; i++) {
        const int kind = tetris.GetPieceKindList(i);
        draw_str(14 + 2 * i, 5, std::to_string(kind).c_str());
    }
    {
        draw_str(15, 20, "NEXT");
        const Point start = {14, 5};

        for (int i = 0; i < 14; i++) {
            const Piece next = tetris.GetNextPiece(i);

            if (next.kind == E)
                continue;

            for (const auto &pos: next.cells) {
                draw_cell(16 + pos.x, 18 + pos.y - i * 3, next.kind);
            }
        }
    }
}

void render()
{
    erase();

    draw_field();
    draw_ghost();
    draw_tetromino();
    draw_info();

    if (tetris.IsGameOver())
        draw_str(1, 11, "GAME OVER");

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

    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        tetris.ChangeTetrominoKind(key - '1' + 1);
        break;

    case 'r':
        frame = 0;
        tetris.PlayGame();
        break;

    case 'q':
        tetris.QuitGame();
        break;

    default:
        break;
    }

    return action;
}
