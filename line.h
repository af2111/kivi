#ifndef LINE_H
#define LINE_H
#include <string>

class Line {
    private:
        std::string text;

    public:
        Line(std::string _text);

        void append(std::string str);
        std::string getText();
        void insert(int index, std::string str);
        void removeChar(int index);
        void erase(int index);
};

#endif