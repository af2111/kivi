
#include "state.h"
#include "text.h"
#include "util.h"


class Screen {
    private:
        State *state;
        Text *text;
    public:
        Screen(State* _state, Text* _text);

        std::string makeWelcomeMsg();

        std::string makeStatusMsg();

        void drawRows();

        void refreshScreen();
};