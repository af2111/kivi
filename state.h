#ifndef STATE_H
#define STATE_H
#include <string>

class State {
    public:
        void setCursorX(int x);
        void setCursorY(int y); 
        int getCursorX();
        int getCursorY();

        std::string getBuff();

        int getEditorMode();
        int setEditorMode(int _mode); 
        int getScreenRows();
        int getScreenCols();
        void setScreenRows(int _rows); 
        void setScreenCols(int _cols);

        State();
        int updateDimensions();
        void buffAppend(std::string val);
        void buffRemove(int index);

        int getBuffX();
        int getBuffY();
        void setBuffX(int x);
        void setBuffY(int y);

        int getOffsetVertical();
        void setOffsetVertical(int _off);
};







#endif