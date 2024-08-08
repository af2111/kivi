
#include "state.h"
#include "tabman.h"
#include "text.h"
#include "util.h"


class Screen {
    private:
        State *state;
        Text *text;
    public:
        Screen(Tabman *_tabman);

        std::string makeWelcomeMsg();

        std::string makeStatusMsg();

        void drawRows();

        void refreshScreen();

        void updateTab();
};