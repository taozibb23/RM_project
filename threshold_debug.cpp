
// threshold_debug.cpp —— 专用 HSV 阈值调试工具
// 用法: ./threshold_debug <图片路径> <red|blue>
// 拖滑块调 HSV，看 mask 实时变化，按 q 退出

#include <opencv2/opencv.hpp>
#include <iostream>

// 全局滑块变量（调试工具专用，允许全局）
int hmin=0, smin=0, vmin=0;
int hmax=179, smax=255, vmax=255;

int main(int argc,char* argv[]){
    if(argc < 3){
        std::cout << "用法： ./threshold_debug <图片路径> <red|blue>"<< std::endl;
        return 1;
    }
    bool usedRed = (std::string(argv[2]) == "red");

    cv::Mat img = cv::imread(argv[1]);
    if(img.empty()){ std::cout<< "获取图片失败"<<std::endl; return 1;}

  // 红色是两段 H：0~10 和 160~179；蓝色一段。这里先用单段，红色问题后面处理
    if (usedRed) { hmin = 160; hmax = 179; }
    else         { hmin = 90;  hmax = 120; }

        cv::namedWindow("TrackBars", cv::WINDOW_NORMAL);
    cv::createTrackbar("H min", "TrackBars", &hmin, 179);
    cv::createTrackbar("H max", "TrackBars", &hmax, 179);
    cv::createTrackbar("S min", "TrackBars", &smin, 255);
    cv::createTrackbar("S max", "TrackBars", &smax, 255);
    cv::createTrackbar("V min", "TrackBars", &vmin, 255);
    cv::createTrackbar("V max", "TrackBars", &vmax, 255);

    while (true) {
        cv::Mat hsv, mask;
        cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, cv::Scalar(hmin, smin, vmin),
                          cv::Scalar(hmax, smax, vmax), mask);
        // 形态学：调参时也能看效果
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7));
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

        cv::imshow("img", img);
        cv::imshow("mask", mask);
        int key = cv::waitKey(30);
        if (key == 'q') break;                    // q 退出
        if (key == 's') {                          // s 打印当前参数（存下来）
            std::cout << "当前: H " << hmin << "~" << hmax
                      << " S " << smin << "~" << smax
                      << " V " << vmin << "~" << vmax << std::endl;
        }
    }
    return 0;
}
