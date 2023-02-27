#include "display.h"

#include <locale.h>
#include <ncurses.h>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <deque>

static const int SCREEN_HEIGHT = FIELD_HEIGHT + 2;
static const int DEFAULT_FG_COLOR = 10;
static const int DEFAULT_BG_COLOR = 11;
static const int DEFAULT_COLOR_PAIR = 10;
static const int CLEARING_DURATION = 20;

Display::Display(Tetris &tetris)
    : tetris_(tetris)
{
    global_offset_ = {1 + 7, 1};
}

Display::~Display()
{
}

int Display::Open()
{
    // Screen
    if (initialize_screen())
        return 1;

    // Timer
    auto start = std::chrono::steady_clock::now();

    tetris_.PlayGame();

    while (tetris_.IsPlaying()) {

        // Input
        const int state = input_key();

        // Game logic
        if (clearing_timer_ == -1)
            tetris_.UpdateFrame(state);

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
        frame_++;

        if (frame_ % 10 == 0)
            fps_ = 1. / elapsed_sec;
    }

    // Clean up
    finalize_screen();

    return 0;
}

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

static const char *get_cell_symbol(int kind)
{
    const char *s = ".";

    switch (kind) {
        case E:
            s = ".";
            break;

        case I: case O: case S: case Z: case J: case L: case T:
            s = "\u25A3"; // solid square
            break;

        case B:
            s = "\u25A1"; // hollow square
            break;

        default:
            s = "\u25A1"; // hollow square
            break;
    }

    return s;
}

int Display::initialize_screen()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (nodelay(stdscr, 1) == ERR) {
        return 1;
    }

    initialize_colors();

    return 0;
}

void Display::finalize_screen()
{
    endwin();
}

void Display::render()
{
    erase();

    draw_borders();
    draw_field();

    draw_ghost();
    draw_tetromino();

    draw_effect();

    draw_info();
    draw_message();
    draw_debug();

    draw_game_over();
    draw_pause();

    refresh();
}

void Display::draw_borders() const
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
            const int kind = tetris_.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
        {
            // Top border
            const int y = FIELD_HEIGHT;
            const int kind = tetris_.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
    }
}

void Display::draw_field() const
{
    // Stack
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            const int kind = tetris_.GetFieldCellKind(Point(x, y));
            draw_cell(x, y, kind);
        }
    }
}

void Display::draw_ghost() const
{
    const bool IS_HOLLOW = true;
    const Piece piece = tetris_.GetGhostPiece();

    if (IsEmptyCell(piece.kind))
        return;

    for (auto pos: piece.cells) {
        draw_cell(pos.x, pos.y, piece.kind, IS_HOLLOW);
    }
}

void Display::draw_tetromino() const
{
    if (clearing_timer_ >=0)
        return;

    if (tetris_.IsGameOver() || tetris_.IsPaused())
        return;

    const Piece piece = tetris_.GetCurrentPiece();

    if (IsEmptyCell(piece.kind))
        return;

    for (auto pos: piece.cells) {
        draw_cell(pos.x, pos.y, piece.kind);
    }
}

void Display::draw_effect()
{
    const int duration = CLEARING_DURATION;
    const int clear_count = tetris_.GetClearedLineCount();

    if (clear_count > 0 && clearing_timer_ == -1)
        clearing_timer_ = duration;
    else if (clearing_timer_ >= 0)
        clearing_timer_--;
    else
        return;

    int cleared_lines[4] = {0};
    tetris_.GetClearedLines(cleared_lines);

    const int frame_per_cell = duration / 5;
    const int erase = clearing_timer_ / frame_per_cell;
    const bool is_flashing = (clear_count == 4) && (clearing_timer_ % 2 == 0);

    // Clear animation
    for (int i = 0; i < clear_count; i++) {
        const int cleared_y = cleared_lines[i];

        for (int x = erase; x < 10 - erase; x++) {
            draw_blank(x, cleared_y, is_flashing);
        }
    }

    // Tetris background effect
    if (is_flashing) {
        for (int y = 0; y < FIELD_HEIGHT; y++) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                const int kind = tetris_.GetFieldCellKind(Point(x, y));
                if (IsEmptyCell(kind))
                    draw_blank(x, y, is_flashing);
            }
        }
    }
}

void Display::draw_info() const
{
    {
        int x = 19, y = 20;

        draw_str(x, y--, "SCORE");
        draw_text(x, y--, "%5d", tetris_.GetScore());

        y--;
        draw_str(x, y--, "LINES");
        draw_text(x, y--, "%5d", tetris_.GetTotalLineCount());

        y--;
        draw_str(x, y--, "LEVEL");
        draw_text(x, y--, "%5d", tetris_.GetLevel());

        y--;
        draw_str(x, y--, "Q: Quit");
        draw_str(x, y--, "R: Reset");
        draw_str(x, y--, "\u2190: Move Left");
        draw_str(x, y--, "\u2192: Move Right");
        draw_str(x, y--, "\u2193: Soft Drop");
        draw_str(x, y--, "Space: Hard Drop");
        draw_str(x, y--, "Z: Spin Left");
        draw_str(x, y--, "X: Spin Right");
        draw_str(x, y--, "C: Hold");
        draw_str(x, y--, "P: Pause");
        draw_str(x, y--, "1: Preview Count");
        draw_str(x, y--, "2: Toggle Ghost");
        draw_str(x, y--, "3: Toggle Hold");
    }
    {
        int x = 13, y = 20;

        draw_str(x, y, "NEXT");

        if (!tetris_.IsPaused()) {
            for (int i = 0; i < 14; i++) {
                const Piece next = tetris_.GetNextPiece(i);

                if (next.kind == E)
                    continue;

                for (const auto &pos: next.cells) {
                    draw_cell(x + pos.x + 1, y + pos.y - 2 - i * 3, next.kind);
                }
            }
        }
    }
    {
        int x = -6, y = 20;

        draw_str(x, y, "HOLD");
        if (!tetris_.IsPaused()) {
            const Piece hold = tetris_.GetHoldPiece();

            if (!IsEmptyCell(hold.kind) && tetris_.IsHoldEnable()) {
                const bool is_hollow = !tetris_.IsHoldAvailable();
                for (const auto &pos: hold.cells) {
                    draw_cell(x + pos.x + 1, y + pos.y - 2, hold.kind, is_hollow);
                }
            }
        }
    }
    {
        int x = 27, y = 20;

        draw_str(x, y--, "FPS");
        draw_text(x, y--, "%g", fps_);
    }
}

void Display::draw_message()
{
    const int line_count = tetris_.GetClearedLineCount();
    const int tspin = tetris_.GetTspinKind();

    const bool send =
        (tspin != TSPIN_NONE && line_count == 0) ||
        (line_count > 0  && clearing_timer_ == 0);

    if (send) {
        if (tetris_.IsPerfectClear() && clearing_timer_ == 0) {
            message_queue_.push_back({"PERFECT CLEAR", frame_});
        }

        if (tspin == TSPIN_NORMAL) {
            message_queue_.push_back({"T-SPIN", frame_});
        }
        else if (tspin == TSPIN_MINI) {
            message_queue_.push_back({"T-SPIN MINI", frame_});
        }

        switch (line_count) {
        case 1: message_queue_.push_back({"SINGLE", frame_}); break;
        case 2: message_queue_.push_back({"DOUBLE", frame_}); break;
        case 3: message_queue_.push_back({"TRIPLE", frame_}); break;
        case 4: message_queue_.push_back({"TETRIS", frame_}); break;
        default: break;
        }

        char buf[64] = {'\0'};
        sprintf(buf, "%+5d", tetris_.GetClearPoints());
        message_queue_.push_back({buf, frame_});
    }

    const int combo_count = tetris_.GetComboCounter();

    if (combo_count > 0 && clearing_timer_ == 0) {
        char buf[64] = {'\0'};
        sprintf(buf, "%d COMBO", combo_count);
        message_queue_.push_back({buf, frame_});
        sprintf(buf, "%+5d", tetris_.GetComboPoints());
        message_queue_.push_back({buf, frame_});
    }

    const int back_to_back = tetris_.GetBackToBackCounter();

    if (back_to_back > 0 && clearing_timer_ == 0) {
        char buf[64] = {'\0'};
        sprintf(buf, "%d BACK TO BACK", back_to_back);
        message_queue_.push_back({buf, frame_});
    }

    message_queue_.erase(
            std::remove_if(
                message_queue_.begin(),
                message_queue_.end(),
                [=](const Message &msg) { return frame_ - msg.start > 60; }),
            message_queue_.end());

    int x = -7, y = 14;
    for (const auto &msg: message_queue_) {
        draw_str(x, y--, msg.str.c_str());
    }
}

void Display::draw_debug() const
{
    if (!tetris_.IsDebugMode())
        return;

    int x = 0, y = -3;
    draw_str(x, y--, "[DEBUG]");
    draw_text(x, y--, "Lock Delay Timer: %d", tetris_.GetLockDelayTimer());
    draw_text(x, y--, "Reset Counter: %d", tetris_.GetResetCounter());
    draw_text(x, y--, "Gravity: %g", tetris_.GetGravity());
    draw_text(x, y--, "Combo Counter: %d", tetris_.GetComboCounter());

    draw_str(x, y, "Next Piece Kinds: ");
    for (int i = 0; i < 14; i++) {
        const int kind = tetris_.GetPieceKindList(i);
        draw_text(x + 18 + i, y, "%d", kind);
    }
}

void Display::draw_game_over()
{
    if (!tetris_.IsGameOver())
        return;

    if (game_over_counter_ == -1)
        game_over_counter_ = 60;
    else if (game_over_counter_ > 0)
        game_over_counter_--;

    const int fill_y = game_over_counter_ / 2;

    for (int y = fill_y; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            draw_str(x, y, "\u25AC");
        }
    }

    if (game_over_counter_ == 0) {
        int x = 0, y = 11;
        draw_str(x, y--, "          ");
        draw_str(x, y--, "GAME OVER ");
        draw_str(x, y--, "          ");
    }
}

void Display::draw_pause() const
{
    if (!tetris_.IsPaused())
        return;

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
            draw_blank(x, y);
        }
    }
    draw_str(2, 10, "PAUSE");
}

int Display::input_key()
{
    const int key = getch();
    int state = 0;

    switch (key) {
    case 'z': case 'd':
        state = ROT_LEFT; break;

    case 'x': case 'f':
        state = ROT_RIGHT; break;

    case KEY_LEFT: case 'h':
        state = MOV_LEFT; break;

    case KEY_RIGHT: case 'l':
        state = MOV_RIGHT; break;

    case KEY_UP: case 'k':
        state = MOV_UP; break;

    case KEY_DOWN: case 'j':
        state = MOV_DOWN; break;

    case ' ': case 'm':
        state = MOV_HARDDROP; break;

    case 'c':
        state = HOLD_PIECE; break;

    case '1':
        {
            const int count = tetris_.GetNextPieceCount();
            tetris_.SetPreviewCount(count == 6 ? 1 : count + 1);
        }
        break;

    case '2':
        tetris_.SetGhostEnable(!tetris_.IsGhostEnable());
        break;

    case '3':
        tetris_.SetHoldEnable(!tetris_.IsHoldEnable());
        break;

    case 9: // TAB
        if (tetris_.IsDebugMode()) {
            const Piece piece = tetris_.GetCurrentPiece();
            int kind = piece.kind;
            kind = kind == 7 ? 1 : kind + 1;
            tetris_.SetTetrominoKind(kind);
        }
        break;

    case 'r':
        frame_ = 0;
        game_over_counter_ = -1;
        clearing_timer_ = -1;
        tetris_.PlayGame();
        break;

    case 'p':
        if (!tetris_.IsGameOver())
            tetris_.PauseGame();
        break;

    case 'q':
        tetris_.QuitGame();
        break;

    default:
        break;
    }

    return state;
}

void Display::draw_str(int x, int y, const char *str) const
{
    const int X = x + global_offset_.x;
    const int Y = y + global_offset_.y;

    mvprintw(SCREEN_HEIGHT - Y - 1, X, str);
}

void Display::draw_cell(int x, int y, int kind, bool is_hollow) const
{
    if (IsEmptyCell(kind) && !tetris_.IsDebugMode())
        return;

    if (IsSolidCell(kind))
        attrset(COLOR_PAIR(kind));
    else
        attrset(COLOR_PAIR(DEFAULT_COLOR_PAIR));

    const char *sym = get_cell_symbol(is_hollow ? -1 : kind);
    draw_str(x, y, sym);

    attrset(0);
}

void Display::draw_blank(int x, int y, bool is_flashing) const
{
    if (is_flashing)
        attron(A_REVERSE);

    draw_str(x, y, " ");

    if (is_flashing)
        attrset(0);
}

void Display::draw_text(int x, int y, const char *str, ...) const
{
    static char buf[256] = {'\0'};
    va_list va;

    va_start(va, str);

    vsprintf(buf, str, va);
    draw_str(x, y, buf);

    va_end(va);
}
