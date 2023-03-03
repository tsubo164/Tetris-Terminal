#ifndef DISPLAY_H
#define DISPLAY_H

#include "tetris.h"
#include <string>
#include <deque>

struct Message {
    Message(const std::string &message, unsigned long start_frame)
        : str(message), start(start_frame)
    {
    }

    std::string str = "";
    unsigned long start = 0;
};

class Display {
public:
    Display(Tetris &tetris);
    ~Display();

    int Open();

private:
    Tetris &tetris_;
    Point global_offset_ = {};
    std::deque<Message> message_queue_;

    int clearing_timer_ = -1;
    int game_over_counter_ = -1;
    unsigned long frame_ = 0;
    float fps_ = 0.f;

    int initialize_screen();
    void finalize_screen();
    int input_key();

    void render();
    void draw_borders() const;
    void draw_field() const;
    void draw_ghost() const;
    void draw_tetromino() const;
    void draw_effect();
    void draw_info() const;
    void draw_message();
    void draw_debug() const;
    void draw_game_over();
    void draw_pause() const;

    void draw_str(int x, int y, const char *str) const;
    void draw_tile(int x, int y, int kind, bool is_hollow = false) const;
    void draw_blank(int x, int y, bool is_flashing = false) const;
    void draw_text(int x, int y, const char *fmt, ...) const;
};

#endif
