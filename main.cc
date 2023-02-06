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
    mvaddch(y, x, ch);
}

static void draw_str(int x, int y, const char *str)
{
    mvaddstr(y, x, str);
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

static void draw_tetromino()
{
    for (int i = 0; i < 4; i++) {
        const Cell cell = GetTetrominoCell(i);
        const char sym = get_cell_symbol(cell.kind);
        draw_char(cell.pos.x, cell.pos.y, sym);
    }
}

static void draw_background()
{
    char line[FIELD_WIDTH + 1] = {'\0'};

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = GetFieldCellKind(x, y);
            line[x] = get_cell_symbol(kind);
        }
        draw_str(0, y, line);
    }
}

static void draw_info()
{
    const std::string fpsstr = "fps: " + std::to_string(fps);
    draw_str(20, 10, fpsstr.c_str());
    draw_str(20, 11, "Press 'Q' to quit");
}

void render()
{
    erase();

    draw_background();
    draw_tetromino();
    draw_info();

    refresh();
}

static int initialize_screen()
{
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

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
