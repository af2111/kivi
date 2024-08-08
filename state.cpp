#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include "state.h"

int cursor_x;
int cursor_y;
std::string buff;
// 0 is normal, 1 is insert, 2 is special (: mode)
int editor_mode;
int screenRows;
int screenCols;

//used for saving cursor position
int buff_x;
int buff_y;

int State::getScreenRows() {
    return screenRows;
}

int State::getScreenCols() {
    return screenCols;
}

void State::setScreenRows(int _rows) {
    screenRows = _rows;
}

void State::setScreenCols(int _cols) {
    screenCols = _cols;
}

int State::updateDimensions() {
    struct winsize ws;
    //Terminal Input/Output Control - Get Window Size
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0 || ws.ws_row == 0) {
        return -1;
    }
    setScreenRows(ws.ws_row);
    setScreenCols(ws.ws_col);

    return 0;
}

State::State() {
    buff = "";
    cursor_x = 0;
    cursor_y = 0;
    editor_mode = 0;
    updateDimensions(); 
}

void State::setCursorX(int x) {
    cursor_x = x;
}
void State::setCursorY(int y) {
    cursor_y = y;
}
int State::getCursorX() {
    return cursor_x;
}
int State::getCursorY() {
    return cursor_y;
}

std::string State::getBuff() {
    return buff;
}

int State::getEditorMode() {
    return editor_mode;
}

int State::setEditorMode(int _mode) {
        if(_mode != 0 && _mode != 1 && _mode != 2) {
            return -1;
        }
        editor_mode = _mode;
        return 0;
}


void State::buffAppend(std::string val) {
    buff.append(val);
}
void State::buffRemove(int index) {
    buff.erase(index);
}

int State::getBuffY() {
    return buff_y;
}

int State::getBuffX() {
    return buff_x;
}

void State::setBuffX(int x) {
    buff_x = x;
}

void State::setBuffY(int y) {
    buff_y = y;
}