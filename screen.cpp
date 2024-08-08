#include "state.h"
#include "text.h"
#include "util.h"
#include "screen.h"

State *screen_state;
Text *text;

Screen::Screen(State* _screen_state, Text* _text) {
    screen_state = _screen_state;
    text = _text;
    if(screen_state->updateDimensions() == -1) {
        die("updateDimensions");
    }

}


std::string Screen::makeWelcomeMsg() {
    std::string MSG = "V - bad vim";
    int padding = (screen_state->getScreenCols() - MSG.length()) / 2;
    std::string spaces(padding+3, ' ');
    return spaces.append(MSG);
}

std::string Screen::makeStatusMsg() {
    std::string explicit_status = screen_state->getStatus();
    std::string filename = screen_state->getFileName();

    int num_spaces = (screen_state->getScreenCols() - (explicit_status.length() + filename.length())) - 1;

    std::string spaces(num_spaces, ' ');
    std::string msg = explicit_status.append(spaces);
    msg.append(filename);
    return msg;
}

void Screen::drawRows() {
    bool show_welcome = true;    
    int offsetVertical = screen_state->getOffsetVertical();
    for(int i = offsetVertical; i < offsetVertical + screen_state->getScreenRows(); i++) {
        Line *current_line = text->getLine(i);
        std::string current_line_text = current_line->getText();
        if(current_line_text != "") {
            screen_state->buffAppend(current_line_text);
            show_welcome = false;
        } else {
            screen_state->buffAppend("~");
            if(i == screen_state->getScreenRows() / 3 && show_welcome) {
                screen_state->buffAppend(makeWelcomeMsg());
            }
        }
        // this clears everything in the current line except the tilde 
        screen_state->buffAppend("\x1b[K"); 

        if(i < (screen_state->getScreenRows() + offsetVertical)) {
            screen_state->buffAppend("\r\n");
        }
        
        if(i == screen_state->getScreenRows() + offsetVertical - 1) {
            screen_state->buffAppend(makeStatusMsg());
            screen_state->buffAppend("\x1b[K"); 
        }
    }
}

void Screen::refreshScreen() {
    //hide cursor while repainting
    screen_state->buffAppend("\x1b[?25l");
    // move cursor to top left
    screen_state->buffAppend("\x1b[H");
    drawRows();
    // make escape sequence to place cursor
    
    std::stringstream cursor_set_msg;
    cursor_set_msg << "\x1b[" << screen_state->getCursorY() + 1 - screen_state->getOffsetVertical() << ";" << screen_state->getCursorX() + 1 << "H";
    screen_state->buffAppend(cursor_set_msg.str());
    // show cursor again
    screen_state->buffAppend("\x1b[?25h");
    //print current buffer
    std::string buff = screen_state->getBuff();
    std::cout << buff;
    //clear current buffer
    state->buffRemove(0);
}

#undef IN_SCREEN_FILE