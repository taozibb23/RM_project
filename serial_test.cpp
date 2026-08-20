#include <iostream>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <cstring>

int main(){
    //1.打开文件(串口)
    //O_RDWR = 可读可写，O_NOCTTY = 不占用终端控制
    int fd = open("/dev/pts/2", O_RDWR | O_NOCTTY);
    if (fd < 0){
        std::cout<<"打开串口失败"<<std::endl;
        return 1;
    }
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
    frame[2] = 0x01;        //x高字节
    frame[3] = 0x40;        //x低字节
    frame[4] = 0x01;        //y高字节
    frame[5] = 0x2E;        //y低字节
    frame[6] = (0xA5 + 0x5A + 0x01 + 0x40 + 0x01 + 0x2E) & 0xFF; //取低  校验位 累加和
    std::cout<<"校验和: 0x" <<std::hex<<(int)frame[6] <<std::dec<<std::endl;

    //发送 write 写文件
    int n = write(fd, frame, 7);
    std::cout<< "发送" <<n<<"字节"<<std::endl;

    close(fd);
    return 0;
}