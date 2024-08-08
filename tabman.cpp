#include "state.h"
#include "text.h"
#include "tabman.h"

int current_tab;
Tab tabs[10];
int max_tabs;
int tabs_used;

Tabman::Tabman() {
    current_tab = 0;
    max_tabs = 10;
    tabs_used = 1;
    State init_state = State();
    Text init_text = Text(&init_state);
    tabs[0].tab_state = &init_state;
    tabs[0].tab_text = &init_text;
}

Tab Tabman::getTab(int index) {
    if(index >= 0 && index < max_tabs) {
        return tabs[index];
    } 
    return Tab {};
}

// returns -1 if max_tabs tabs are already used
int Tabman::addTab() {
    if(tabs_used == max_tabs) {
        return -1;
    }
    State new_state = State();
    Text new_text = Text(&new_state);
    tabs[tabs_used].tab_state = &new_state;
    tabs[tabs_used].tab_text = &new_text;
    tabs_used++;
    return 0;
}

int Tabman::getCurrentTab() {
    return current_tab;
}

void Tabman::nextTab() {
    if(current_tab + 1 == tabs_used) {
        addTab();
    }
    ++current_tab;
}

void Tabman::prevTab() {
    --current_tab;
}