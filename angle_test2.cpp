// angle_test2.cpp —— 只验证 getBarDir 和配对角度逻辑
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

double normalizeDeg(double angle){
    while(angle > 100.0) angle -= 360.0;
    while(angle < -100.0)angle += 360.0;
    return angle;
}

double getBarDir(const cv::RotatedRect& r){
        if(r.size.width >= r.size.height) return r.angle;
        else return r.angle;
}

int main() {
    // 模拟"两根竖直灯条"：一个 angle=0（宽=长边），一个 angle=90（高=长边）
    cv::RotatedRect bar1(cv::Point2f(100,100), cv::Size2f(10,50), 0);    // 宽10高50：高是长边
    cv::RotatedRect bar2(cv::Point2f(150,100), cv::Size2f(50,10), 90);   // 宽50高10：宽是长边

    double d1 = normalizeDeg(getBarDir(bar1));
    double d2 = normalizeDeg(getBarDir(bar2));
    std::cout << "bar1 长边方向: " << d1 << std::endl;
    std::cout << "bar2 长边方向: " << d2 << std::endl;
    std::cout << "方向差: " << std::abs(d1 - d2) << "  → " 
              << (std::abs(d1-d2) < 15 ? "✅ 判定平行" : "❌ 判定不平行") << std::endl;
    return 0;
}