//
// Created by LEI XU on 4/27/19.
//
#include "Texture.hpp"
#include <eigen3/Eigen/Eigen>

/// @brief 这里的 u, v 在区间 [0, 1] 中
/// @param u 横坐标
/// @param v 纵坐标
/// @return 返回纹理颜色
Eigen::Vector3f Texture::getColorBilinear(float u, float v) {
    // 找到 (u, v) 临近的四个点
    float x = u * (width - 1);
    float y = (1 - v) * (height - 1);
    int left = std::max(static_cast<int>(std::floor(x)), 0);
    int right = std::min(left + 1, width - 1);
    int bottom = std::max(static_cast<int>(std::floor(y)), 0);
    int top = std::min(bottom + 1, height - 1);
    // 四个点的纹理颜色
    auto c00 = image_data.at<cv::Vec3b>(bottom, left);
    auto c01 = image_data.at<cv::Vec3b>(top, left);
    auto c10 = image_data.at<cv::Vec3b>(bottom, right);
    auto c11 = image_data.at<cv::Vec3b>(top, right);
    Eigen::Vector3f color00(c00[0], c00[1], c00[2]);
    Eigen::Vector3f color01(c01[0], c01[1], c01[2]);
    Eigen::Vector3f color10(c10[0], c10[1], c10[2]);
    Eigen::Vector3f color11(c11[0], c11[1], c11[2]);
    // 双线性插值，看成权重就好理解了，离谁更近权重就越大
    float s = x - left, t = y - bottom;
    Eigen::Vector3f color0 = color00 * (1.f - s) + color10 * s;
    Eigen::Vector3f color1 = color10 * (1.f - t) + color11 * t;
    
    return color0 * (1.f - t) + color1 * t;
}

