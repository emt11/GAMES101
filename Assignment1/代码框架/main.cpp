#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

/// @brief 绕 Z 轴进行逆时针旋转
/// @param rotation_angle 旋转角度
/// @return 旋转矩阵
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    Eigen::Matrix4f rotation;
    float angle = rotation_angle * std::numbers::pi / 180.0; // 获取弧度
    rotation << std::cos(angle), -std::sin(angle), 0, 0,
                std::sin(angle), std::cos(angle), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;

    model = rotation * model;

    return model;
}

/// @brief 绕任意轴旋转
/// @param axis 轴
/// @param angle 旋转角度
/// @return 旋转矩阵
Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    axis.normalize();
    float radian = angle * std::numbers::pi / 180.0;
    Eigen::Matrix3f N;
    N << 0.0f, -axis.z(), axis.y(),
         axis.z(), 0.0f, -axis.x(),
         -axis.y(), axis.x(), 0;

    const Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f rotation = 
        std::cos(radian) * I + 
        (1.0f - std::cos(radian)) * axis * axis.transpose() + 
        std::sin(radian) * N;

    Eigen::Matrix4f result = Eigen::Matrix4f::Identity();
    result.block<3, 3>(0, 0) = rotation;

    return result;
}

/// @brief 投影变换(Projection)
/// @param eye_fov // 垂直可视角度
/// @param aspect_ratio XY 平面的宽高比
/// @param zNear 相机离近平面的 Z 轴距离
/// @param zFar 相机离原平面的 Z 轴距离
/// @return 投影变换矩阵
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f perspective, orthographic_scale, orthographic_transfrom;
    perspective << zNear, 0, 0, 0,
                   0, zNear, 0, 0,
                   0, 0, zNear + zFar, -zNear * zFar,
                   0, 0, 1, 0;
    
    float xL, xR, yT, yB;
    float forY = eye_fov * std::numbers::pi / 180.0;
    yT = std::tan(forY / 2) * std::abs(zNear);
    yB = -yT;
    xR = yT * aspect_ratio;
    xL = -xR;

    orthographic_scale << 2 / (xR - xL), 0, 0, 0,
                          0, 2 / (yT - yB), 0, 0,
                          0, 0, 2 / (zNear - zFar), 0,
                          0, 0, 0, 1;
    orthographic_transfrom << 1, 0, 0, -(xR + xL) / 2,
                              0, 1, 0, -(yT + yB) / 2,
                              0, 0, 1, -(zNear + zFar) / 2,
                              0, 0, 0, 1;

    projection = orthographic_scale * orthographic_transfrom * perspective * projection;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    // get arguments
    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700); // 定义一个分辨率为 700 * 700 的光栅器

    Eigen::Vector3f eye_pos = {0, 0, 5}; // 相机坐标

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}}; // 物体顶点，可以有多个坐标
 
    std::vector<Eigen::Vector3i> ind{{0, 1, 2}}; // 三角形顶点索引

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // 使用命令行参数，直接输出在文件中
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth); // 清空上一帧的颜色和深度

        // MVP 变换
        // r.set_model(get_model_matrix(angle));
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) { // Esc == 27 ，即 Esc 退出循环
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        // r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation({1, 1, 0}, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10); // 等待 10 ms

        std::cout << "frame count: " << frame_count++ << '\n';

        // 旋转角度
        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
