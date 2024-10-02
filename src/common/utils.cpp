#include "utils.hpp"

#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>

namespace MCA2 {
std::string getPath(const std::string &pathPattern, int idx) {
    char filePath[256];
    std::snprintf(filePath, sizeof(filePath), pathPattern.c_str(), idx);
    return std::string(filePath);
}

std::string pointToString(const cv::Point &point) {
    return std::to_string(point.x) + "," + std::to_string(point.y);
}

cv::Point stringToPoint(const std::string &str) {
    size_t commaPos = str.find(",");
    int x = std::stoi(str.substr(0, commaPos));
    int y = std::stoi(str.substr(commaPos + 1));

    return cv::Point(x, y);
}

double calculateDistance(const cv::Point &point1, const cv::Point &point2) {
    return std::sqrt(std::pow(point2.x - point1.x, 2) + std::pow(point2.y - point1.y, 2));
}

double calculateSSIM(const cv::Mat &img1, const cv::Mat &img2) {
    cv::Mat img1_gray, img2_gray;
    cv::cvtColor(img1, img1_gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(img2, img2_gray, cv::COLOR_BGR2GRAY);

    // 确保两幅图像大小相同
    assert(img1_gray.size() == img2_gray.size());

    // 图像转换为浮点型
    cv::Mat img1_f, img2_f;
    img1_gray.convertTo(img1_f, CV_32F);
    img2_gray.convertTo(img2_f, CV_32F);

    // 定义 SSIM 计算所需的变量
    cv::Mat mu1, mu2, sigma1, sigma2, sigma12;
    double C1 = 6.5025, C2 = 58.5225;

    // 计算均值 (mu)
    cv::GaussianBlur(img1_f, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(img2_f, mu2, cv::Size(11, 11), 1.5);

    // 计算平方 (sigma)
    cv::Mat mu1_mu1 = mu1.mul(mu1);
    cv::Mat mu2_mu2 = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);

    // 计算方差 (sigma)
    cv::GaussianBlur(img1_f.mul(img1_f), sigma1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(img2_f.mul(img2_f), sigma2, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(img1_f.mul(img2_f), sigma12, cv::Size(11, 11), 1.5);

    sigma1 -= mu1_mu1;
    sigma2 -= mu2_mu2;
    sigma12 -= mu1_mu2;

    // 计算 SSIM
    cv::Mat ssim_map = ((2 * mu1_mu2 + C1).mul(2 * sigma12 + C2)) /
                       ((mu1_mu1 + mu2_mu2 + C1).mul(sigma1 + sigma2 + C2));

    // 取平均值
    cv::Scalar mssim = cv::mean(ssim_map);
    return mssim[0];
}

// 判断字符串中有没有通配符
bool hasFormatSpecifier(const std::string &str) {
    static const std::string formatSpecifiers = "cdfsfeEgGxXo";

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%') {
            if (i + 1 < str.length() && formatSpecifiers.find(str[i + 1]) != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

cv::Mat expandImage(const cv::Mat &srcImage, int top, int left, int bottom, int right) {
    cv::Mat res(cv::Size(srcImage.cols + left + right, srcImage.rows + top + bottom),
                srcImage.type(), cv::Scalar(0, 0, 0));
    srcImage.copyTo(res(cv::Rect(left, top, srcImage.cols, srcImage.rows)));
    return res;
}

cv::Mat cropImage(const cv::Mat &srcImage, int top, int left, int bottom, int right) {
    return srcImage(
        cv::Rect(left, top, srcImage.cols - left - right, srcImage.rows - top - bottom));
}
} // namespace MCA2