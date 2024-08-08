/*** INCLUDES  ***/

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include "util.h"
#include "terminal.h"
#include "state.h"

/*** GLOBALS ***/


class Line {
    private:
        std::string text;
    public:
        Line(std::string _text) {
            text = _text;
        }
        void append(std::string str) {
            text.append(str);
        }
        std::string getText() {
            return text;
        } 
        void insert(int index, std::string str) {
            text.insert(index, str);
        }
        void removeChar(int index) {
            text.erase(index, 1);
        }
        void erase(int index) {
            text.erase(index);
        }
        
};

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
            return &lines.at(state->getCursorY());
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
        void scroll_back() {
            
        }
        void clear() {
            for(int i = 0; i < max_line; i++) {
                getLine(i)->erase(0);
            }
        }
        void openFile(std::string path) {
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
                
        }
};

/*** INPUT HANDLER CLASS ***/

class InputHandler {
    private:
        char current_char;
        State *state;
        Text *text;
    public:
        InputHandler(State *_state, Text *_text) {
            state = _state;
            text = _text;
        }

        void getChar() {
            // if(read(STDIN_FILENO, &current_char, 1) != 1) {
            //     die("read getChar");
            // }
            std::cin.get(current_char);    
        }

        void goDown(State *state, Text *text) {        
            if(state->getCursorY() + 1 >= state->getScreenRows() + state->getOffsetVertical()) {
                state->setOffsetVertical(state->getOffsetVertical() + 1);
                text->addLine();
            }
            state->setCursorY(state->getCursorY() + 1);
            state->setCursorX(text->getLine(state->getCursorY() + 1)->getText().length());
        }
        void handleChar() {
            int current_state = state->getEditorMode();
            int currentY = state->getCursorY();
            int currentX = state->getCursorX();
            Line *current_line = text->getCurrentLine();
            int max_rows = state->getScreenRows();
            int max_cols = state->getScreenCols();
            // these are used to put the cursor back after exiting colon mode
            int buff_x = state->getBuffX();
            int buff_y = state->getBuffY();
            int vertical_offset = state->getOffsetVertical();
            switch(current_state) {
                case 0:
                    switch(current_char) {
                        case 'i':
                            state->setEditorMode(1);
                            text->getLine((max_rows + vertical_offset) - 1)->append("INSERT");
                            break;
                        case ':':
                            state->setBuffX(currentX);
                            state->setBuffY(currentY);
                            text->getLine((max_rows + vertical_offset) - 1)->append(":");
                            state->setCursorY((max_rows + vertical_offset) - 1);
                            state->setCursorX(1);
                            state->setEditorMode(2);
                            break;
                        case '\n':
                        case 'j':
                            if(currentY + 1 >= max_rows + vertical_offset) {
                                state->setOffsetVertical(state->getOffsetVertical() + 1);
                                text->addLine();
                            }
                            state->setCursorY(currentY + 1);
                            state->setCursorX(text->getLine(currentY + 1)->getText().length());
                            break;
                        case 'k':
                            if(currentY != 0) {
                                if(currentY - 1 <= vertical_offset) {
                                    state->setOffsetVertical(vertical_offset - 1);
                                }
                                state->setCursorY(currentY - 1);
                                state->setCursorX(text->getLine(currentY - 1)->getText().length());
                            }
                            break;
                        case 'h':
                            if(currentX != 0) {
                                state->setCursorX(currentX - 1);
                            }
                            break;
                        case 'l':
                            if(currentX + 1 < max_cols) {
                                state->setCursorX(currentX + 1);
                            }
                            break;
                    }
                        
                    break;
                case 1:
                    switch(current_char) {
                        case 27: 
                            state->setEditorMode(0);
                            text->getLine((max_rows + vertical_offset)-1)->erase(0);
                            break;
                        case '\n':
                            if(currentX == current_line->getText().length()) {
                                state->setCursorY(currentY + 1);
                                state->setCursorX(text->getLine(currentY + 1)->getText().length());
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

                            }
                            current_line->erase(currentX - 1);
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
                            state->setEditorMode(0);
                            current_line->erase(0);
                            state->setCursorX(buff_x);
                            state->setCursorY(buff_y);
                            break;
                        case '\n':
                            //get command text without colon
                            {
                                std::string command_text = current_line->getText().substr(1);
                                state->setEditorMode(0);
                                current_line->erase(0);
                                state->setCursorX(buff_x);
                                state->setCursorY(buff_y);
                                if(command_text == "q") {
                                    quitRegular();
                                    break;
                                }
                                if(command_text == "w") {
                                    text->writeFile("text");
                                }
                                if(command_text == "o") {
                                    text->openFile("text");
                                }
                            }
                            break;
                        default:
                            state->setCursorX(currentX + 1);
                            current_line->insert(currentX, std::string(1, current_char));
                            break;
                        
                        
                            
                    }
                    break;
            }
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

                if(i < (state->getScreenRows() + offsetVertical) - 1) {
                    state->buffAppend("\r\n");
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