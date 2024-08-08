#ifndef TABMAN_H
#define TABMAN_H
#include "state.h"
#include "text.h"

typedef struct Tab {
    State *tab_state;
    Text *tab_text;
} Tab;

class Tabman {
    public:
        Tabman();

        Tab getTab(int index);
        int addTab();
        int getCurrentTab();
        void setCurrentTab(int sel);
        void nextTab();
        void prevTab();
};

#endif