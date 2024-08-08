#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include "terminal.h"


struct termios original_settings;

void resetTerminal() {
        if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_settings) == -1) {
            die("tcsetattr resetTerminal");
        }
}

Terminal::Terminal() {
            if(tcgetattr(STDIN_FILENO, &original_settings) == -1) {
                die("tcgetattr constructor");
            }
            atexit(resetTerminal);
}

void Terminal::enableRawMode() {
        struct termios raw_mode_terminal = original_settings;
        // IXON disables CTRL-S and CTRL-Q
        raw_mode_terminal.c_iflag &= ~(IXON | BRKINT | INPCK | ISTRIP);
        // echo is for displaying input, icanon is canonical mode, isig is ctrl c / ctrl z, IEXTEN is ctrl v
        raw_mode_terminal.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN );
        // disable output processing
        raw_mode_terminal.c_oflag &= ~(OPOST);
        
        raw_mode_terminal.c_cflag |= (CS8);

        if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode_terminal) == -1) {
            die("tcsetattr enableRawMode");
        }
}

