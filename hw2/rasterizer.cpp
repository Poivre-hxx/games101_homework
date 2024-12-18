// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <Eigen>


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
    // 三条边按照clockwise
    Vector3f point = { static_cast<float>(x), static_cast<float>(y), 0.0f };  // 使用浮点数并设置Z为0
    Vector3f v01 = _v[1] - _v[0];
    Vector3f v0p = point - _v[0];
    Vector3f v12 = _v[2] - _v[1];
    Vector3f v1p = point - _v[1];
    Vector3f v20 = _v[0] - _v[2];
    Vector3f v2p = point - _v[2];

    float p0 = v01.x() * v0p.y() - v01.y() * v0p.x();  // 叉乘的2D版本
    float p1 = v12.x() * v1p.y() - v12.y() * v1p.x();
    float p2 = v20.x() * v2p.y() - v20.y() * v2p.x();

    if ((p0 >= 0 && p1 >= 0 && p2 >= 0) || (p0 <= 0 && p1 <= 0 && p2 <= 0)) {
        return true;
    }
    return false;

    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
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
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    // 第4个维度填充为1
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float min_x = width;
    float max_x = 0;
    float min_y = height;
    float max_y = 0;

    /*
    for (int i = 0; i < 3; i++) {
        if (v[i].x() < min_x) min_x = v[i].x();
        if (v[i].x() > max_x) max_x = v[i].x();
        if (v[i].y() < min_y) min_y = v[i].y();
        if (v[i].y() > max_x) max_y = v[i].y();
    }
    */
    for (auto point : v) {
        min_x = MIN(point[0], min_x);
        max_x = MAX(point[0], max_x);
        min_y = MIN(point[1], min_y);
        max_y = MAX(point[1], max_y);
    }

    for (int x = min_x; x < max_x; x++) {
        for (int y = min_y; y < max_x; y++) {
            
            /*
            //z-buffer
            if (insideTriangle((float)x + 0.5, (float)y + 0.5, t.v)) {
                // If so, use the following code to get the interpolated z value.
                // abg是一个三元组类型的变量
                auto abg = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
                auto alpha = std::get<0>(abg);
                auto beta = std::get<1>(abg);
                auto gamma = std::get<2>(abg);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;

                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                int index = get_index(x, y);
                if (z_interpolated <= depth_buf[index]) {
                    Eigen::Vector3f p;
                    p << x, y, z_interpolated;
                    set_pixel(p, t.getColor());
                    depth_buf[index] = z_interpolated;
                }
            }
            */

            ///*
            // msaa
            float per = 0;
            if (insideTriangle((float)x + 0.25, (float)y + 0.25, t.v)) per += 0.25;
            if (insideTriangle((float)x + 0.25, (float)y + 0.75, t.v)) per += 0.25;
            if (insideTriangle((float)x + 0.75, (float)y + 0.25, t.v)) per += 0.25;
            if (insideTriangle((float)x + 0.75, (float)y + 0.75, t.v)) per += 0.25;
            // If so, use the following code to get the interpolated z value.
            // abg是一个三元组类型的变量
            if (per >= 0.25) {
                auto abg = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
                auto alpha = std::get<0>(abg);
                auto beta = std::get<1>(abg);
                auto gamma = std::get<2>(abg);
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;

                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                int index = get_index(x, y);
                if (z_interpolated < depth_buf[index]) {
                    Eigen::Vector3f p;
                    p << x, y, z_interpolated;
                    set_pixel(p, t.getColor() * per);
                    depth_buf[index] = z_interpolated;
                }
            }
            //*/
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
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

// 初始化深度缓冲和帧缓冲
rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
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