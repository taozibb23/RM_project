#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char* argv[]){
    if (argc < 2){
        std::cout<<"用法 : ./img <图片路径>"<<std::endl;
        return 1;
    }
    cv::Mat img = cv::imread(argv[1]);
    if(img.empty()){
        std::cout<<"img get error :"<<argv[1]<<std::endl;
        return 1;
    }
    std::cout << "size = "<< img.cols<<" x "<<img.rows<<std::endl;
    cv::imshow("image",img);
    cv::waitKey(0);
    return 0;
};