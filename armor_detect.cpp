#include <opencv2/opencv.hpp>
#include <iostream>

// 阈值：trackbar 调参结果（写死）
int hminb = 72,  sminb = 16,  vminb = 255;
int hmaxb = 101, smaxb = 69,  vmaxb = 255;

int hminr = 153, sminr = 8,   vminr = 255;
int hmaxr = 179, smaxr = 61,  vmaxr = 255;

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

    cv::Scalar lower(hmin, smin, vmin);   
    cv::Scalar upper(hmax, smax, vmax);
    cv::inRange(imgHSV, lower, upper, imgmask);
    cv::morphologyEx(imgmask, imgmask, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarcy;
    cv::Mat boxPtsMat; 
    std::vector<cv::Point> boxPtsInt;
    cv::findContours(imgmask, contours,hierarcy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for(int i = 0;i <contours.size(); i++){
        double area = cv::contourArea(contours[i]);
        std::cout<<"contours  :  "<<i<<"area  :  "<<area<<std::endl;
        if (cv::contourArea(contours[i]) > 30 ){
            cv::RotatedRect rRect = cv::minAreaRect(contours[i]);
            cv::boxPoints(rRect,boxPtsMat);
            std::vector<cv::Point2f> boxPts;
            for( int k = 0; k < boxPtsMat.rows; k++){
                boxPts.push_back(cv::Point2f(boxPtsMat.at<float>(k,0),
                                 boxPtsMat.at<float>(k,1)));
            }
            boxPtsInt.clear();
            for(int k = 0; k<4;k++){
                boxPtsInt.push_back(cv::Point(boxPts[k].x, boxPts[k].y));
            }
            cv::polylines(img,boxPtsInt,true,cv::Scalar(0,255,0), 2);
            std::cout<<"灯带角度"<<rRect.angle<<"灯带中心"<<rRect.center<<std::endl;
        }
    }

    cv::imshow("img", img);
    cv::setMouseCallback("img", onMouse, &imgHSV);
    cv::imshow("imgmask", imgmask);
    cv::waitKey(0);
    return 0;
}
