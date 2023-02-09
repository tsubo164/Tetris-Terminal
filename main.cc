#include "tetris.h"

#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static int initialize_screen();
static void finalize_screen();
static void input_key();
static void render();

static const int SCREEN_HEIGHT = FIELD_HEIGHT;

// Counters
static double fps = 0.0;
static unsigned long frame = 0;

int main(int argc, char **argv)
{
    // Arguments
    if (argc == 2) {
        if (!strcmp(argv[1], "-d")) {
            SetDebugMode();
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

    PlayGame();

    while (IsPlaying()) {

        // Input
        input_key();

        // Game logic
        UpdateFrame();

        // Rednering
        render();;

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

static char get_cell_symbol(int cell)
{
    switch (cell) {
        case E: return ' ';
        case I: return 'I';
        case O: return 'O';
        case S: return 'S';
        case Z: return 'Z';
        case J: return 'J';
        case L: return 'L';
        case T: return 'T';
        case B: return 'B';
        default: return ' ';
    }
}

static void draw_cell(int kind, int x, int y)
{
    if (kind == E)
        return;

    const char sym = get_cell_symbol(kind);
    attrset(COLOR_PAIR(kind));
    draw_char(x, y, sym);
    attrset(0);
}

static void draw_tetromino()
{
    for (int i = 0; i < 4; i++) {
        const Cell cell = GetTetrominoCell(i);
        draw_cell(cell.kind, cell.pos.x, cell.pos.y);
    }
}

static void draw_field()
{
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = GetFieldCellKind(Point(x, y));
            draw_cell(kind, x, y);
        }
    }

    const int clearing_timer = GetClearingTimer();
    if (clearing_timer == -1)
        return;

    int cleared_lines[4] = {0};
    GetClearedLines(cleared_lines);
    const int duration = 20;
    const int frame_per_cell = duration / 5;
    const int erase = clearing_timer / frame_per_cell;

    for (int i = 0; i < 4; i++) {
        const int cleared_y = cleared_lines[i];

        if (cleared_y == 0)
            continue;

        for (int x = erase + 1; x < 11 - erase; x++) {
            draw_char(x, cleared_y, ' ');
        }
    }
}

static void draw_info()
{
    const std::string fpsstr = "fps: " + std::to_string(fps);
    draw_str(20, 11, fpsstr.c_str());
    draw_str(20, 10, "Press 'Q' to quit");
}

void render()
{
    erase();

    draw_field();
    draw_tetromino();
    draw_info();

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
    init_pair(B + 1, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
    bkgd(COLOR_PAIR(B + 1));

    assign_color(I, 0, 1000, 1000);
    assign_color(O, 1000, 1000, 0);
    assign_color(S, 0, 1000, 0);
    assign_color(Z, 1000, 0, 0);
    assign_color(J, 100, 300, 1000);
    assign_color(L, 1000, 500, 0);
    assign_color(T, 1000, 0, 500);
    assign_color(B, 800, 800, 800);
}

static int initialize_screen()
{
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

static void input_key()
{
    const int key = getch();

    switch (key) {
    case 'd': MoveTetromino(ROT_LEFT); break;
    case 'f': MoveTetromino(ROT_RIGHT); break;
    case 'h': MoveTetromino(MOV_LEFT); break;
    case 'l': MoveTetromino(MOV_RIGHT); break;
    case 'k': MoveTetromino(MOV_UP); break;
    case 'j': MoveTetromino(MOV_DOWN); break;

    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        ChangeTetrominoKind(key - '1' + 1);
        break;

    case 'r':
        frame = 0;
        PlayGame();
        break;

    case 'q':
        QuitGame();
        break;

    default:
        break;
    }
}
