#include "tetris.h"
#include "display.h"

#include <cstdio>
#include <cstring>

int main(int argc, char **argv)
{
    Tetris tetris;
    Display display(tetris);

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

    return display.Open();
}
