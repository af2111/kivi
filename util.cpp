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

