#include "armor_algo.hpp"

double ArmorDetect::normalizeDeg(double angle){
    while(angle > 180.0) angle -= 360.0;
    while(angle < -180.0) angle += 360.0;
    return angle;
}

double ArmorDetect::getBarDir(const cv::RotatedRect& r){
    if (r.size.width >= r.size.height) return r.angle;
    else return r.angle + 90.0;
}

bool ArmorDetect::isValidPair(const cv::RotatedRect& a, const cv::RotatedRect& b, cv::RotatedRect& armor_out){
    double heightratio = a.size.height / b.size.height;
    if (heightratio < 0.67 || heightratio > 1.5) return false;

    double a_dir = normalizeDeg(getBarDir(a));
    double b_dir = normalizeDeg(getBarDir(b));
    double dir_dif = std::abs(a_dir - b_dir);
    if (dir_dif > 90) dir_dif = 180 - dir_dif;
    if (dir_dif > 15) return false;

    double centerdis = std::hypot(a.center.x - b.center.x, a.center.y - b.center.y);
    double avgH = (a.size.height + b.size.height) / 2.0;
    double ratio = centerdis / avgH;
    if (ratio < 1.5 || ratio > 4.0) return false;

    double center_deg = std::atan2(a.center.y - b.center.y, a.center.x - b.center.x) * 180.0 / CV_PI;
    double bar_deg = (getBarDir(a) + getBarDir(b)) / 2.0;
    double dev = std::abs(std::abs(normalizeDeg(bar_deg - center_deg)) - 90.0);
    if (dev > 20) return false;

    // 配对成功：两个灯条 8 个角点合并成装甲板矩形
    std::vector<cv::Point2f> pts;
    cv::Mat ma, mb;
    cv::boxPoints(a, ma);
    cv::boxPoints(b, mb);
    for (int k = 0; k < ma.rows; k++)
        pts.push_back(cv::Point2f(ma.at<float>(k,0), ma.at<float>(k,1)));
    for (int k = 0; k < mb.rows; k++)
        pts.push_back(cv::Point2f(mb.at<float>(k,0), mb.at<float>(k,1)));
    armor_out = cv::minAreaRect(pts);
    return true;
}

ArmorType ArmorDetect::classifyArmor(const cv::RotatedRect& armor){
    double w = armor.size.width, h = armor.size.height;
    if (w < h) std::swap(w, h);          // w 是长边
    double ratio = w / h;                // 长宽比
    if (ratio > 2.0)  return ArmorType::SMALL;   // 细长 → 小装甲板
    if (ratio > 1.5)  return ArmorType::MEDIUM;  // 中等
    return ArmorType::BIG;                        // 接近方形 → 大装甲板
}
