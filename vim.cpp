/*** INCLUDES  ***/

#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <variant>
#include <vector>
#include <fstream>
#include "util.h"
#include "terminal.h"
#include "state.h"
#include "line.h"
/*** GLOBALS ***/


class Text {
    private:
        State *state;
        std::vector<Line> lines;
        int max_line;
    public:
        Text(State *_state) {
            state = _state;
            for(int i = 0; i < state->getScreenRows(); i++) {
                lines.push_back(Line(""));
                max_line = i;
            }
        }
        // returns -1 if line doesnt exist
        int appendToLine(int index, std::string str) {
            lines.at(index).append(str);
            return 0;
        }
        Line *getCurrentLine() {
            if(state->getCursorY() > max_line) {
                return NULL;
            }

            Line *ret = &lines.at(state->getCursorY());
            return ret;
        }
        Line *getLine(int index) {
            if(index <= max_line) {
                return &lines.at(index);
            }
            return NULL;
        }

        void writeFile(std::string path) {
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

        void addLine() {
            max_line++;
            lines.push_back(Line(""));
        }
        void clear() {
            for(int i = 0; i < max_line; i++) {
                getLine(i)->erase(0);
            }
        }
        int openFile(std::string path) {
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

        int getMaxLine() {
            return max_line;
        }

        // returns index of first line containing a string (starting from start), or -1, if not found
        int findStr(std::string search, int start) {
            for(int i = start; i <= max_line; i++) {
                if(lines[i].getText().find(search) != std::variant_npos) {
                    return i;
                }
            }
            return -1;
        }
};

/*** INPUT HANDLER CLASS ***/

class InputHandler {
    private:
        char current_char;
        State *state;
        Text *text;

        void goDown() {        
            if(state->getCursorY() + 1 >= state->getScreenRows() + state->getOffsetVertical()) {
                state->setOffsetVertical(state->getOffsetVertical() + 1);
                text->addLine();
            }
            state->setCursorY(state->getCursorY() + 1);
            state->setCursorX(text->getLine(state->getCursorY())->getText().length());
        }

        void goUp() {
            if(state->getCursorY() != 0) {
                if(state->getCursorY() - 1 <= state->getOffsetVertical()) {
                    state->setOffsetVertical(state->getOffsetVertical() - 1);
                }
                state->setCursorY(state->getCursorY() - 1);
                state->setCursorX(text->getLine(state->getCursorY())->getText().length());
            }
        }

        void enterInsertMode() {
            state->setEditorMode(1);
            state->setStatus("INSERT");
        }

        void enterNormalMode() {
            state->setEditorMode(0);
            state->setStatus("NORMAL");
        }

        void enterColonMode() {
            state->setStatus(":");
            state->setCursorY((state->getScreenCols() + state->getOffsetVertical()));
            state->setCursorX(1);
            state->setEditorMode(2);
        }

        void saveCursorPos() {
            state->setBuffX(state->getCursorX());
            state->setBuffY(state->getCursorY());
            state->setBuffOffset(state->getOffsetVertical());
        }

        void restoreCursorPos() {
            state->setCursorX(state->getBuffX());
            state->setCursorY(state->getBuffY());
            state->setOffsetVertical(state->getBuffOffset());
        }

        int statusBackspace() {
            std::string status = state->getStatus();
            if(state->getCursorX() == 1) {
                return -1;
            }
            state->setStatus(status.substr(0, status.length() - 1));
            state->setCursorX(state->getCursorX() - 1);
            return 0;
        }

        void statusWriteChar(char _char) {
            state->setCursorX(state->getCursorX() + 1);
            state->setStatus(state->getStatus().append(std::string(1, _char)));
        }

    public:
        InputHandler(State *_state, Text *_text) {
            state = _state;
            text = _text;
        }

        void getChar() {
            std::cin.get(current_char);    
        }

        void handleChar() {
            int currentY = state->getCursorY();
            int currentX = state->getCursorX();
            Line *current_line = text->getCurrentLine();
            // these are used to put the cursor back after exiting colon mode
            switch(state->getEditorMode()) {
                case 0:
                    switch(current_char) {
                        case 'i':
                            enterInsertMode();
                            break;
                        case ':':
                            saveCursorPos();
                            enterColonMode();
                            break;
                        case '\n':
                        case 'j':
                            goDown();
                            break;
                        case 'k':
                            goUp();
                            break;
                        case 'h': 
                            if(currentX != 0) {
                                state->setCursorX(currentX - 1);
                            } else {
                                goUp();
                            }
                            break;
                        case 'l':
                            if(currentX >= current_line->getText().length()) {
                                break;
                            }
                            state->setCursorX(currentX + 1);
                            break;
                        case '$':
                            state->setCursorX(current_line->getText().length());
                            break;
                    }
                        
                    break;
                case 1:
                    switch(current_char) {
                        case 27: 
                            enterNormalMode();
                            break;
                        case '\n':
                            if(currentX == current_line->getText().length()) {
                                goDown();
                            } else {
                                if(currentY + 1 < state->getScreenRows()) {
                                    std::string part_1 = current_line->getText().substr(0, currentX);
                                    std::string part_2 = current_line->getText().substr(currentX);
                                    current_line->erase(currentX);
                                    text->getLine(currentY + 1)->insert(0, part_2);
                                    state->setCursorY(currentY + 1);
                                    state->setCursorX(0);
                                }
                            }
                            break;
                        // backspace
                        case 127:
                        case '\b':
                            if(currentX == 0) {
                                goUp();
                                break;
                            }
                            current_line->removeChar(currentX - 1);
                            state->setCursorX(currentX - 1);
                            break;
                        default:
                            state->setCursorX(currentX + 1);
                            current_line->insert(currentX, std::string(1, current_char));
                            break;
                            
                    }
                    break;
                case 2:
                    switch(current_char) {
                        case 27:
                            enterNormalMode();
                            restoreCursorPos();
                            break;
                        case '\n':
                            //get command text without colon
                            {
                                std::string command_text = state->getStatus().substr(1);
                                enterNormalMode();
                                restoreCursorPos();
                                if(command_text == "q") {
                                    quitRegular();
                                    break;
                                }
                                if(command_text.substr(0, 1) == "w") {
                                    if(command_text == "w") {
                                        if(state->getFileName() != "") {
                                            text->writeFile(state->getFileName());
                                        } else {
                                            state->setStatus("Filename required!");
                                        }
                                    }
                                    if(command_text.substr(1, 1) == " ") {
                                        state->setFileName(command_text.substr(2));
                                        text->writeFile(state->getFileName());
                                    }
                                }
                                if(command_text.substr(0, 2) == "o ") {
                                    state->setFileName(command_text.substr(2));
                                    if(text->openFile(state->getFileName()) == -1) {
                                        state->setStatus("File doesn't exist!");
                                    }
                                }
                                if(command_text.substr(0, 1) == "s") {
                                    state->setSearch(command_text.substr(2));
                                    state->setSearchRes(text->findStr(state->getSearch(), 0));
                                    saveCursorPos();
                                    if(state->getSearchRes() == -1) {
                                        state->setStatus("Not Found!");
                                        break;
                                    }
                                    state->setOffsetVertical(state->getOffsetVertical() + computeAdditionalOffset(state->getSearchRes(), state->getOffsetVertical(), state->getScreenCols()));
                                    state->setCursorY(state->getSearchRes());
                                    state->setEditorMode(3);

                                }
                                break;
                            }
                            break;
                        case 127:
                            statusBackspace();
                            break;
                        default:
                            statusWriteChar(current_char);
                            break;
                    }
                    break;

                case 3:
                    int start = state->getSearchRes();
                    int res = text->findStr(state->getSearch(), start + 1);
                    state->setSearchRes(res);
                    if(res == -1) {
                        state->setStatus("break");
                        state->setSearchRes(text->findStr(state->getSearch(), 0));
                        state->setOffsetVertical(state->getSearchRes());
                        state->setCursorY(state->getSearchRes());
                        break;
                    }
                    state->setCursorY(res);
                    int offsetNeeded = computeAdditionalOffset(res, state->getOffsetVertical(), state->getScreenRows());
                    if(offsetNeeded != 0) {
                        state->setOffsetVertical(state->getOffsetVertical() + computeAdditionalOffset(state->getSearchRes(), state->getOffsetVertical(), state->getScreenRows()) + state->getScreenRows());
                    }
                    state->setStatus(std::to_string(offsetNeeded)); 
                    switch(current_char) {
                        case 'n':
                            break;
                        case '\n':
                            state->setCursorY(start);
                            saveCursorPos();
                            enterNormalMode();
                            break;
                        case 27:
                            restoreCursorPos();
                            enterNormalMode();
                            break;
                    }
                    

            }
        }
};

class StatusBar {
    public:
        std::string content;
        StatusBar() {
            content = "NORMAL";
        }       
};

class Screen {
    private:
        State *state;
        Text *text;
    public:
        Screen(State* _state, Text* _text) {
            state = _state;
            text = _text;
            if(state->updateDimensions() == -1) {
                die("updateDimensions");
            }

        }

        
        std::string makeWelcomeMsg() {
            std::string MSG = "V - bad vim";
            int padding = (state->getScreenCols() - MSG.length()) / 2;
            std::string spaces(padding+3, ' ');
            return spaces.append(MSG);
        }

        std::string makeStatusMsg() {
            std::string explicit_status = state->getStatus();
            std::string filename = state->getFileName();

            int num_spaces = (state->getScreenCols() - (explicit_status.length() + filename.length())) - 1;

            std::string spaces(num_spaces, ' ');
            std::string msg = explicit_status.append(spaces);
            msg.append(filename);
            return msg;
        }

        void drawRows() {
            bool show_welcome = true;    
            int offsetVertical = state->getOffsetVertical();
            for(int i = offsetVertical; i < offsetVertical + state->getScreenRows(); i++) {
                Line *current_line = text->getLine(i);
                std::string current_line_text = current_line->getText();
                if(current_line_text != "") {
                    state->buffAppend(current_line_text);
                    show_welcome = false;
                } else {
                    state->buffAppend("~");
                    if(i == state->getScreenRows() / 3 && show_welcome) {
                        state->buffAppend(makeWelcomeMsg());
                    }
                }
                // this clears everything in the current line except the tilde 
                state->buffAppend("\x1b[K"); 

                if(i < (state->getScreenRows() + offsetVertical)) {
                    state->buffAppend("\r\n");
                }
                
                if(i == state->getScreenRows() + offsetVertical - 1) {
                    state->buffAppend(makeStatusMsg());
                    state->buffAppend("\x1b[K"); 
                }
            }
        }

        void refreshScreen() {
            //hide cursor while repainting
            state->buffAppend("\x1b[?25l");
            // move cursor to top left
            state->buffAppend("\x1b[H");
            drawRows();
            // make escape sequence to place cursor
            
            std::stringstream cursor_set_msg;
            cursor_set_msg << "\x1b[" << state->getCursorY() + 1 - state->getOffsetVertical() << ";" << state->getCursorX() + 1 << "H";
            state->buffAppend(cursor_set_msg.str());
            // show cursor again
            state->buffAppend("\x1b[?25h");
            //print current buffer
            std::string buff = state->getBuff();
            std::cout << buff;
            //clear current buffer
            state->buffRemove(0);
        }

};





/*** MAIN ***/

int main() {
    State state = State();
    Text text = Text(&state);
    Terminal terminal = Terminal();
    terminal.enableRawMode();
    InputHandler inputHandler = InputHandler(&state, &text);
    Screen screen = Screen(&state, &text);
    while (true) {
        screen.refreshScreen();
        inputHandler.getChar();
        inputHandler.handleChar();
    }
    return 0;
}
