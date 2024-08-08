/*** INCLUDES  ***/

#include <cstring>
#include <sstream>
#include <string>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include "terminal.h"
#include "state.h"
#include "line.h"
#include "text.h"
#include "screen.h"
/*** GLOBALS ***/



/*** INPUT HANDLER CLASS ***/

class InputHandler {
    private:
        char current_char;
        State *state;
        Text *text;
        Screen *screen;

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

        
        void goToNextRes() {
            int start = state->getSearchRes();
            int res = text->findStr(state->getSearch(), start + 1);
            state->setSearchRes(res);
            if(res == -1) {
                int first_res = text->findStr(state->getSearch(), 0);
                state->setCursorY(first_res);
                state->setOffsetVertical(first_res);
                screen->refreshScreen();
                state->setSearchRes(first_res);
                return;
            }
            int text_maxline = text->getMaxLine();
            int space_taken_after_res = text_maxline - res;
            if(space_taken_after_res < state->getScreenRows()) {
                for(int i = 0; i < state->getScreenRows() - space_taken_after_res; i++) {
                    text->addLine();
                }
            }
            state->setCursorY(res);
            state->setOffsetVertical(res);
            screen->refreshScreen();
        }

    public:
        InputHandler(State *_state, Text *_text, Screen *_screen) {
            state = _state;
            text = _text;
            screen = _screen;
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
                                    goToNextRes();
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
                    switch(current_char) {
                        case 'n':
                            goToNextRes();
                            break;
                        case '\n':
                            state->setCursorY(state->getCursorY());
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




/*** MAIN ***/

int main() {
    State main_state = State();
    Text text = Text(&main_state);
    Terminal terminal = Terminal();
    terminal.enableRawMode();
    Screen screen = Screen(&main_state, &text);
    InputHandler inputHandler = InputHandler(&main_state, &text, &screen);
    while (true) {
        screen.refreshScreen();
        inputHandler.getChar();
        inputHandler.handleChar();
    }
    return 0;
}
