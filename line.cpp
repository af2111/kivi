#include <string>
#include "line.h"

std::string text;
Line::Line(std::string _text) {
    text = _text;
}
void Line::append(std::string str) {
    text.append(str);
}
std::string Line::getText() {
    return text;
} 
void Line::insert(int index, std::string str) {
    text.insert(index, str);
}
void Line::removeChar(int index) {
    text.erase(index, 1);
}
void Line::erase(int index) {
    text.erase(index);
}