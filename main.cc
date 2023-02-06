#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "tetris.h"

static void render();

int main(int argc, char **argv)
{
    initscr();
    cbreak();
    noecho();

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    unsigned long frame = 0;
    double fps = 0.0;
    auto start = std::chrono::steady_clock::now();

    PlayGame();

    while (IsPlaying()) {

        // Input
        const int key = getch();

        switch (key) {
        case 'd': MoveTetromino(ROT_LEFT); break;
        case 'f': MoveTetromino(ROT_RIGHT); break;
        case 'h': MoveTetromino(MOV_LEFT); break;
        case 'l': MoveTetromino(MOV_RIGHT); break;
        case 'k': MoveTetromino(MOV_UP); break;
        case 'j': MoveTetromino(MOV_DOWN); break;
        case 'q': QuitGame(); break;
        default: break;
        }

        // Game logic
        UpdateFrame();

        // Rednering
        erase();

        render();;

        const std::string fpsstr = "fps: " + std::to_string(fps);
        mvaddstr(10, 20, fpsstr.c_str());
        mvaddstr(11, 20, "Press 'Q' to quit");

        refresh();

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

    endwin();

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

void render()
{
    char line[FIELD_WIDTH + 1] = {'\0'};

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = GetFieldCellKind(x, y);
            line[x] = get_cell_symbol(kind);
        }
        draw_str(0, y, line);
    }

    draw_tetromino();
}
