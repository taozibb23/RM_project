#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <cstring>

#include "armor_type.hpp"
#include "armor_algo.hpp"
#include "serial.hpp"
// 阈值：trackbar 调参结果 不要写死，把全部数据先i塞进来再进行下一步
// ===== 阈值参数
//蓝色
int hminb = 88,  sminb = 9,  vminb = 255;
int hmaxb = 101, smaxb = 49,  vmaxb = 255;
//红色
int hminr = 0, sminr = 11,   vminr = 255;
int hmaxr = 26, smaxr = 71,  vmaxr = 255;
int hminr2 = 0,  sminr2 = 11, vminr2 = 255;
int hmaxr2 = 10, smaxr2 = 71, vmaxr2 = 255;
//装甲板类型

// 鼠标采样：点图打印该像素 HSV（不点不影响运行）
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        cv::Mat* hsv = (cv::Mat*)userdata;
        cv::Vec3b p = hsv->at<cv::Vec3b>(y, x);
        std::cout << "(" << x << "," << y << ") H=" << (int)p[0]
                  << " S=" << (int)p[1] << " V=" << (int)p[2] << std::endl;
    }
}

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        std::cout << "用法: ./armor_detect <图片路径>" << std::endl;
        return 1;
    }

    int serialFd = openSerial("/dev/pts/2"); //根据实际串口参数不对的话o要改完u保存重新编译

while(true){
    cv::Mat img = cv::imread(argv[1]);

    if (img.empty()) {
        std::cout << "img get error: " << argv[1] << std::endl;
        return 1;
    }

    bool usedRed = (argc >= 3 && std::string(argv[2]) == "red");
    int hmin = usedRed ? hminr : hminb;
    int smin = usedRed ? sminr : sminb;
    int vmin = usedRed ? vminr : vminb;
    int hmax = usedRed ? hmaxr : hmaxb;
    int smax = usedRed ? smaxr : smaxb;
    int vmax = usedRed ? vmaxr : vmaxb;

    ArmorDetect armordetect;
    cv::Mat imgHSV, imgmask;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7));
    
        double t0 = cv::getTickCount();  //帧开始的时间

    cv::cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);
    if(usedRed){
        cv::Mat mask1, mask2;
        cv::inRange(imgHSV, cv::Scalar(hminr2, sminr, vminr), cv::Scalar(hmaxr2, smaxr, vmaxr), mask1);
        cv::inRange(imgHSV, cv::Scalar(hminr, sminr, vminr), cv::Scalar(hmaxr, smaxr, vmaxr), mask2);
        cv::bitwise_or(mask1, mask2, imgmask);
    }else{
            cv::Scalar lower(hmin, smin, vmin);   
            cv::Scalar upper(hmax, smax, vmax);
            cv::inRange(imgHSV, lower, upper, imgmask);
         }
    cv::morphologyEx(imgmask, imgmask, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarcy;
    cv::Mat boxPtsMat; 
    std::vector<cv::Point> boxPtsInt;
    std::vector<cv::RotatedRect> lightBars;
    cv::findContours(imgmask, contours,hierarcy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    
    for(int i = 0;i <contours.size(); i++){
        double area = cv::contourArea(contours[i]);
        //std::cout<<"contours  :  "<<i<<"area  :  "<<area<<std::endl;
        if (cv::contourArea(contours[i]) > 30){
            cv::RotatedRect rRect = cv::minAreaRect(contours[i]);
            double h = rRect.size.height, w = rRect.size.width;
            if(h < w)std::swap(h,w);
            if(h / w < 3 )continue;//长宽比
            cv::boxPoints(rRect,boxPtsMat);
            std::vector<cv::Point2f> boxPts;
            lightBars.push_back(cv::minAreaRect(contours[i]));
            for( int k = 0; k < boxPtsMat.rows; k++){
                boxPts.push_back(cv::Point2f(boxPtsMat.at<float>(k,0),
                                             boxPtsMat.at<float>(k,1)));
            }
            boxPtsInt.clear();
            for(int k = 0; k<4;k++){
                boxPtsInt.push_back(cv::Point(boxPts[k].x, boxPts[k].y));
            }
            cv::polylines(img,boxPtsInt,true,cv::Scalar(0,255,0), 2);
            if(lightBars.empty()){std::cout<<" 没有检测到灯带 "<<std::endl;}
            std::cout<<"灯带角度"<<(rRect.angle + 90)<<"灯带中心"<<rRect.center<<std::endl;//angle加90  全部都以长边为标准
            //std::cout<<"检测到灯带： "<<lightBars.size() << " 个 "<<std::endl;
        }
     
    }
           //----------------------筛选灯带数量----------------
            std::cout<<"正在配对灯带中....."<<std::endl;
            std::sort(lightBars.begin(), lightBars.end(),
                    [](const cv::RotatedRect& a, const cv::RotatedRect& b){
                        return a.center.x < b.center.x;
                    });
            //筛选出角度垂直一些的灯带
            for(auto it = lightBars.begin(); it != lightBars.end();){
                double longdir = armordetect.normalizeDeg(it->angle + 90);//规划一
                if(std::abs(longdir - 110.0) > 55)
                    it = lightBars.erase(it);
                else
                    ++it;
            }
            cv::RotatedRect armor;//接受配对完的装甲板
            for(std::size_t i = 0; i < lightBars.size(); i++){
                for(std::size_t j = i + 1 ; j < lightBars.size(); j++){//i为第一个e灯带j为第二个灯带
                    if(armordetect.isValidPair(lightBars[i],lightBars[j],armor)){//配对成功
                        std::cout<<"配对 成功 "<<i<<" + "<<j<<std::endl;
                        ArmorType armortype = armordetect.classifyArmor(armor);
                        switch (armortype)
                        {
                        case ArmorType::BIG:   std::cout<<"装甲板类型:BIG"  <<std::endl;break;
                        case ArmorType::SMALL: std::cout<<"装甲板类型:SMALL"<<std::endl;break;
                        default:               std::cout<<"装甲板类型:UNKNOWN"<<std::endl;break;    
                        }
                        // 用 isValidPair 返回的 armor 矩形直接画框
                        cv::Mat armorPtsMat;
                        cv::boxPoints(armor, armorPtsMat);
                        std::vector<cv::Point> armorPtsInt;
                        for(int k = 0; k < armorPtsMat.rows; k++){
                            float x = armorPtsMat.at<float>(k,0);
                            float y = armorPtsMat.at<float>(k,1);
                            armorPtsInt.push_back(cv::Point(cvRound(x), cvRound(y)));
                        }
                        cv::polylines(img, armorPtsInt, true, cv::Scalar(0,255,0), 2, cv::LINE_AA);

                        int send_x = (int)armor.center.x;
                        int send_y = (int)armor.center.y;
                        int send_type = (armortype == ArmorType::BIG) ? 1 : 0;//BIG就是1  SMALL就是0
                        sendFrame(serialFd, send_x, send_y, send_type);
                        
                        double dist = armordetect.getDistance(armor);
                        std::cout<<"距离:"<<dist<<"mm"<<std::endl;

                    }else std::cout<<"配对失败"<<std::endl;

                }
            
            }

       
    
    double t1 = cv::getTickCount();
    double fps = cv::getTickFrequency() / (t1 - t0);//计算帧率
    std::cout<<"FPS:"<<fps<<std::endl;
    std::string fpsText = "FPS:" + std::to_string(static_cast<int>(fps));
    cv::putText(img,fpsText,cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(0,255,0),2,cv::LINE_AA);
    cv::imshow("img", img);
    cv::setMouseCallback("img", onMouse, &imgHSV);
     
    cv::imshow("imgmask", imgmask);

     
     if(cv::waitKey(1) == 'q')break;  //q推出

    }//while的
    close(serialFd);
    return 0;
}
