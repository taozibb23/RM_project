#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "armor_type.hpp"

class ArmorDetect{
public:
double normalizeDeg(double angle);
double getBarDir(const cv::RotatedRect& r);
bool isValidPair(const cv::RotatedRect& a,const cv::RotatedRect& b, cv::RotatedRect& armor_out);
ArmorType classifyArmor(const cv::RotatedRect& armor);
double getDistance(const cv::RotatedRect& armor)const;//测距离

};