//serial_test.hpp
#pragma once


int openSerial(const char* path);
void sendFrame(int fd,int x, int y, int type);
