// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector2f P(x, y);
    Vector2f A(_v[0].x(), _v[0].y());
    Vector2f B(_v[1].x(), _v[1].y());
    Vector2f C(_v[2].x(), _v[2].y());
    // 使用叉乘判断是否在一个三角形内部
    auto cross = [](const Vector2f& u, const Vector2f& v) -> float {
        return u.x() * v.y() - u.y() * v.x();
    };
    float c1 = cross(A - B, A - P);
    float c2 = cross(B - C, B - P);
    float c3 = cross(C - A, C - P);
    return (c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0  && c2 <= 0 && c3 <= 0);
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            // t.setVertex(i, v[i].head<3>());
            // t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
    // 再做一次颜色平均
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int pixel_idx = (height - 1 - y) * width + x;
            Eigen::Vector3f color_sum{0, 0, 0};
            for (int i = 0; i < row_sample_rate; ++i) {
                for (int j = 0; j < col_sample_rate; ++j) {
                    int sample_id = j * col_sample_rate + i;
                    int ss_idx = pixel_idx * sample_count + sample_id;
                    color_sum += ss_color_buf[ss_idx];
                }
            }
            frame_buf[pixel_idx] = color_sum / static_cast<float>(sample_count);
        }
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
    // 最简单的方案：找出三个顶点对应 x y 的最大最小值就行了
    auto v = t.toVector4();
    float left = width - 1, right = 0, bottom = height - 1, top = 0;
    Vector4f v0 = v[0], v1 = v[1], v2 = v[2];
    left = std::floor(std::min({left, v0.x(), v1.x(), v2.x()}));
    right = std::ceil(std::max({right, v0.x(), v1.x(), v2.x()}));
    bottom = std::floor(std::min({bottom, v0.y(), v1.y(), v2.y()}));
    top = std::ceil(std::max({top, v0.y(), v1.y(), v2.y()}));
    // 防止有负数
    left   = std::max(0.0f, left);
    right  = std::min((float)(width - 1), right);
    bottom = std::max(0.0f, bottom);
    top    = std::min((float)(height - 1), top);
    // 遍历每个像素点，判断是否在三角形内部（在边上认为不是内部）
    // 实现 super-sampling 2*2
    for (int x = left; x <= right; ++x) {
        for (int y = bottom; y <= top; ++y) {
            int pixel_idx = (height - 1 - y) * width + x;
            for (int i = 0; i < row_sample_rate; ++i) {
                for (int j = 0; j < col_sample_rate; ++j) {
                    // 获取像素点中心
                    // float cx = x + (1.0f / 2 / row_sample_rate) + 1.0f / row_sample_rate * i;
                    // float cy = y + (1.0f / 2 / col_sample_rate) + 1.0f / col_sample_rate * i;
                    float cx = x + (0.5f + i) / col_sample_rate;
                    float cy = y + (0.5f + j) / row_sample_rate;
                    if (!insideTriangle(cx, cy, t.v)) continue;
                    // 重心插值
                    auto[alpha, beta, gamma] = computeBarycentric2D(cx, cy, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    // 更新 Z-Buffer，原点在左上角
                    int ss_idx = pixel_idx * sample_count + j * col_sample_rate + i;
                    if (z_interpolated > ss_depth_buf[ss_idx]) {
                        ss_depth_buf[ss_idx] = z_interpolated;
                        ss_color_buf[ss_idx] = t.getColor();
                    }
                }
            }

            // if (!insideTriangle(x + 0.5, y + 0.5, t.v)) continue;
            // // 重心插值
            // auto[alpha, beta, gamma] = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
            // float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            // float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            // z_interpolated *= w_reciprocal;
            // // 更新 Z-Buffer，原点在左上角
            // if (z_interpolated > depth_buf[pixel_idx]) {
            //     depth_buf[pixel_idx] = z_interpolated;
            //     set_pixel(Vector3f{float(x), float(y), 0.0f}, t.getColor());   
            // }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(ss_color_buf.begin(), ss_color_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), -std::numeric_limits<float>::infinity());
        std::fill(ss_depth_buf.begin(), ss_depth_buf.end(), -std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    ss_depth_buf.resize(w * h * sample_count);
    ss_color_buf.resize(w * h * sample_count);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on