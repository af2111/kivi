#include <string>
#include <iostream>

     
void die(std::string err) {
    //write(STDOUT_FILENO, &"\x1b[2J", 4);
    //write(STDOUT_FILENO, "\x1b[H", 3);
    std::cout << "\x1b[2J";
    std::cout << "\x1b[H";
    std::cerr << err;
    exit(1);
}

void quitRegular() {
    std::cout << "\x1b[2J";
    std::cout << "\x1b[H";

    exit(0);
}

int computeAdditionalOffset(int line, int current_offset, int rows) {
    if(line < current_offset + rows && line >= current_offset) {
        return 0;
    }

    if(line >= current_offset + rows) {
        return line - (current_offset + rows);
    }

    if(line < current_offset) {
        return -(current_offset - line);
    }

    
    return 0;
}

