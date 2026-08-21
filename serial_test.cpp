#include <iostream>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <cstring>

#include "serial.hpp"

int openSerial(const char* path){
    int fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0){
        std::cout<<"打开串口失败"<<std::endl;
        return 1;
    }
    return fd;
}

void sendFrame(int fd,int x, int y, int type){
    unsigned int value_x = x;
    unsigned int value_y = y;

    unsigned char frame_input[4];
    frame_input[0] = value_x & 0xFF;
    frame_input[1] = (value_x >> 8) & 0xFF;
    frame_input[2] = value_y & 0xFF;
    frame_input[3] = (value_y >> 8) & 0xFF;

    std::cout<<"输入的数值: 0x"<<std::hex<< value_x <<value_y << std::endl;
    std::cout<<"frame_output:";
    for(int i = 0;i < 4;i++){
        std::cout<<"0x"<<std::hex<<(int)frame_input[i]<<"  ";
    }
    std::cout<< std::endl;

        //2.配置串口
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    tcgetattr(fd, &tty);

    cfsetispeed(&tty, B1152000);      //配置输入输出波特率
    cfsetospeed(&tty, B1152000);
    tty.c_cflag |= (CLOCAL | CREAD);  //启动接收
    tty.c_cflag &= ~CSIZE;            //清除数据位
    tty.c_cflag |= CS8;               //8 位数据位
    tty.c_cflag &= ~PARENB;           //无校验
    tty.c_cflag &= ~CSTOPB;           //1 个停止位
    tcsetattr(fd, TCSANOW, &tty);     //立即生效

    //构造一帧  A5 5A | x=320(01 40) | y=302(01 2E) | 校验
    unsigned char frame[7];
    frame[0] = 0xA5;        //帧头1
    frame[1] = 0x5A;        //帧头2
    //前面接受的是从低字节到高字节 到这里输出帧要一样也就是 把前面变成先高后低
    frame[2] = frame_input[1];        //x高字节
    frame[3] = frame_input[0];        //x低字节
    frame[4] = frame_input[3];        //y高字节
    frame[5] = frame_input[2];        //y低字节
    frame[6] = (frame[0]+frame[1]+frame[2]+frame[3]+frame[4]+frame[5]) & 0xFF; //取低  校验位 累加和
    std::cout<<"校验和: 0x" <<std::hex<<(int)frame[6] <<std::dec<<std::endl;

    //发送 write 写文件
    int n = write(fd, frame, 7);
    std::cout<< "发送" <<n<<"字节"<<std::endl;

    close(fd);

}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "用法: " << argv[0] << " <x> <y>" << std::endl;
        return 1;
    }
    int fd = openSerial("/dev/pts/2");
    if (fd < 0) return 1;
    sendFrame(fd, std::stoi(argv[1]), std::stoi(argv[2]), 0);
    close(fd);
    return 0;
}