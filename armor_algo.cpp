#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "armor_type.hpp"

double normalizeDeg(double angle){//灯带配对
    while(angle > 180.0) angle -= 360.0;
    while(angle < -180.0) angle += 360.0;
    return angle;
}
double getBarDir(const cv::RotatedRect& r){
    if(r.size.width >= r.size.height)
        return r.angle;
    else
        return r.angle + 90;
}
bool isValidPair(const cv::RotatedRect& a,const cv::RotatedRect& b, cv::RotatedRect armor_out){
     auto heightratio = a.size.height/b.size.height;
     //条件1 高度比例
     if (heightratio < 0.67 || heightratio > 1.5){
          std::cout<<"高度比例不正确 "<<"位置在 :"<<a.center<<"  "<<b.center<<std::endl;
          return false;}//高度比例  if早退模式里面写反条件
          double a_dir = normalizeDeg(getBarDir(a));
          double b_dir = normalizeDeg(getBarDir(b));
          //要注意归一化的hi时候以什么为基准，绕圈和翻折
          double dir_dif = std::abs(normalizeDeg(a_dir - b_dir));
          if (dir_dif > 90)dir_dif = 180 - dir_dif;
    //条件2 两个灯带的长边 角度相差在一定范围内
          if(dir_dif > 15)return false;//绝对值  std  abs
            std::cout<<"angle : "<<(a.angle + 90)<<"   "<<(b.angle + 90)<<std::endl;
            auto centerspacing_x = a.center.x - b.center.x;//间距是负数怎么办
            auto centerspacing_y = a.center.y - b.center.y;
            auto centerdis = std::hypot(centerspacing_x,centerspacing_y);//计算间距
            auto averageheight = (a.size.height + b.size.height) / 2 ;
    //条件3 组合起来的矩形用center连线和height 矩形的比例在一定范围
            if(4 < centerdis / averageheight || centerdis / averageheight < 1.5)return false;//中心间距除以平均u高度来筛选
               auto dx = a.center.x - b.center.x;
               auto dy = a.center.y - b.center.y;
               auto center_angle_rad = std::atan2(dy,dx);
               auto center_angle_deg = center_angle_rad *180 / CV_PI;  //弧度转度数
               auto judgmentangle = 90; //......可能不要
               std::cout<<" angle :"<<center_angle_deg<<std::endl;
               auto angle_a = (a.angle + 90.0); //angle是nn短边的夹角+90nh变成aa长边了
               auto angle_b = (b.angle + 90.0);
               auto two_bar_deg = (angle_a + angle_b) / 2;//取得平均值
               two_bar_deg = normalizeDeg(two_bar_deg);
               center_angle_deg = normalizeDeg(center_angle_deg);
               auto bar_center_dif = two_bar_deg - center_angle_deg;
               bar_center_dif = normalizeDeg(bar_center_dif);
               auto deviation = std::abs(std::abs(bar_center_dif) - 90.0);//与垂直的偏差
               float tolerance_deg = 20.0;//+-  误差范围是20
    //条件4  中心center连线与灯带垂直 误差在一定范围内                
                    if(deviation > tolerance_deg)return false;//中心连线和灯带的角度差
                        std::cout<<"配对成功"<<std::endl;
                        return true;                                                
        
}
ArmorType classifyArmor(const cv::RotatedRect& armor){
    auto armor_ratio = armor.size.width / armor.size.height;
    if(armor_ratio > 3.0){
        return ArmorType::BIG;
    }
    if(armor_ratio < 2.5){
        return ArmorType::SMALL;
    }
    return ArmorType::SMALL;
}
