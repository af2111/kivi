#ifndef TEXT_H
#define TEXT_H

#include "state.h"
#include <variant>
#include <vector>
#include "line.h"
#include <fstream>
#include <filesystem>

class Text {
    public:
        Text(State *_state);
        int appendToLine(int index, std::string str);
        Line *getCurrentLine();
        Line *getLine(int index);
        void writeFile(std::string path);
        void addLine();
        void clear();
        int openFile(std::string path);
        int getMaxLine();
        int findStr(std::string search, int start);
};






#endif