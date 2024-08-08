#include "text.h"
#include "state.h"
#include <variant>
#include <vector>
#include "line.h"
#include <fstream>
#include <filesystem>


State *state;
std::vector<Line> lines;
int max_line;


Text::Text(State *_state) {
    state = _state;
    for(int i = 0; i < state->getScreenRows(); i++) {
        lines.push_back(Line(""));
        max_line = i;
    }
}
// returns -1 if line doesnt exist
int Text::appendToLine(int index, std::string str) {
    lines.at(index).append(str);
    return 0;
}
Line *Text::getCurrentLine() {
    if(state->getCursorY() > max_line) {
        return NULL;
    }

    Line *ret = &lines.at(state->getCursorY());
    return ret;
}
Line *Text::getLine(int index) {
    if(index <= max_line) {
        return &lines.at(index);
    }
    return NULL;
}

void Text::writeFile(std::string path) {
    int last_text_line;
    for(int i = 0; i <= max_line; i++) {
        if(lines.at(i).getText() != "") {
            last_text_line = i;
        }
    }

    std::ofstream write_file;
    write_file.open(path);
    
    for(int i = 0; i <= last_text_line; i++) {
        write_file << lines.at(i).getText() << "\n";
    }
    write_file.close();

}

void Text::addLine() {
    max_line++;
    lines.push_back(Line(""));
}
void Text::clear() {
    for(int i = 0; i < max_line; i++) {
        getLine(i)->erase(0);
    }
}
int Text::openFile(std::string path) {
    if(!std::filesystem::exists(path)) {
        return -1;
    }
    clear();
    std::ifstream file;
    file.open(path);
    std::string current_line;
    int i = 0;
    while(std::getline(file, current_line)) {
        if(i > max_line) {
            addLine();
        }
        getLine(i)->append(current_line);
        i++;
    }


    return 0;
        
}

int Text::getMaxLine() {
    return max_line;
}

// returns index of first line containing a string (starting from start), or -1, if not found
int Text::findStr(std::string search, int start) {
    for(int i = start; i <= max_line; i++) {
        if(lines[i].getText().find(search) != std::variant_npos) {
            return i;
        }
    }
    return -1;
}