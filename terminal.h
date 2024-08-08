#ifndef TERMINAL_H
#define TERMINAL_H

#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"



class Terminal {

    public:
        Terminal();
        void enableRawMode(); 
};

#endif
