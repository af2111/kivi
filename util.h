#ifndef UTIL_H
#define UTIL_H
#include <iostream>
#include <string>   

void die(std::string err);   
void quitRegular();
int computeAdditionalOffset(int line, int current_offset, int rows);

#endif
