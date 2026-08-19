#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

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
enum class ArmorType{
    SMALL,
    BIG,
    UNKNOWN
};
// 鼠标采样：点图打印该像素 HSV（不点不影响运行）
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        cv::Mat* hsv = (cv::Mat*)userdata;
        cv::Vec3b p = hsv->at<cv::Vec3b>(y, x);
        std::cout << "(" << x << "," << y << ") H=" << (int)p[0]
                  << " S=" << (int)p[1] << " V=" << (int)p[2] << std::endl;
    }
}
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "用法: ./armor_detect <图片路径>" << std::endl;
        return 1;
    }
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
 
    cv::Mat imgHSV, imgmask;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7));
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
                double longdir = normalizeDeg(it->angle + 90);//规划一
                if(std::abs(longdir - 110.0) > 55)
                    it = lightBars.erase(it);
                else
                    ++it;
            }
            cv::RotatedRect armor;//接受配对完的装甲板
            for(std::size_t i = 0; i < lightBars.size(); i++){
                for(std::size_t j = i + 1 ; j < lightBars.size(); j++){//i为第一个e灯带j为第二个灯带
                    if(isValidPair(lightBars[i],lightBars[j],armor)){//配对成功
                        std::cout<<"配对 成功 "<<i<<" + "<<j<<std::endl;
                        ArmorType armortype = classifyArmor(armor);
                        switch (armortype)
                        {
                        case ArmorType::BIG:   std::cout<<"装甲板类型:BIG"  <<std::endl;break;
                        case ArmorType::SMALL: std::cout<<"装甲板类型:SMALL"<<std::endl;break;
                        default:               std::cout<<"装甲板类型:UNKNOWN"<<std::endl;break;    
                        }
                        cv::Mat mi,mj;
                        cv::boxPoints(lightBars[i],mi);
                        cv::boxPoints(lightBars[j],mj);//获取灯带的四个点
                        std::vector<cv::Point2f> allPts;
                        for(int i = 0;i < mi.rows; i++){//找出mi的列数 这里的mat是4x2的矩阵
                            allPts.push_back(cv::Point2f(mi.at<float>(i,0),mi.at<float>(i,1)));
                        }
                        for(int j = 0;j < mj.rows; j++){
                            allPts.push_back(cv::Point2f(mj.at<float>(j,0),mj.at<float>(j,1)));
                        }
                        auto armorpts = cv::minAreaRect(allPts);//i最小矩形
                        cv::Mat armorPtsMat;//用math接受后面转换成int
                        cv::boxPoints(armorpts,armorPtsMat);
                        std::vector<cv::Point> armorPtsInt;//储存int类型的
                        for(int i = 0;i < armorPtsMat.rows;i++){
                            float x = armorPtsMat.at<float>(i,0);
                            float y = armorPtsMat.at<float>(i,1);
                            armorPtsInt.push_back(cv::Point(cvRound(x),cvRound(y)));
                            
                        }
                        if(armorPtsInt.empty()){std::cout<<" 没有装甲板可以绘制"<<std::endl;}
                        cv::polylines(img,armorPtsInt,true,cv::Scalar(0,255,0),2,cv::LINE_AA);
                    }else std::cout<<"配对失败"<<std::endl;

                }
            
            }

       
    
   
     cv::imshow("img", img);
     cv::setMouseCallback("img", onMouse, &imgHSV);
     cv::imshow("imgmask", imgmask);
     cv::waitKey(0);
     return 0;

}
