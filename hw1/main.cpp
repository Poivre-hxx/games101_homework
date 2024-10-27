#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace Eigen;

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float ang = rotation_angle / 180.0 * acos(-1);
    Eigen::Matrix4f rotate;
    rotate << cos(ang), -sin(ang), 0, 0,
        sin(ang), cos(ang), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    model = rotate * model;

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f persp;
    Eigen::Matrix4f ortho;

    persp << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, (-1.0 * zNear * zFar),
        0, 0, 1, 0;

    double w, h, z;
    h = zNear * tan(eye_fov / 2) * 2;
    w = h * aspect_ratio;
    z = zFar - zNear;

    ortho << 2 / w, 0, 0, 0,
        0, 2 / h, 0, 0,
        0, 0, 2 / z, -(zFar + zNear) / 2,
        0, 0, 0, 1;

    projection = ortho * persp * projection;

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate1, translate2, trans;
    float rotx = atan2(axis[1], axis[2]);
    translate1 << 1, 0, 0, 0,
        0, cos(rotx), -sin(rotx), 0,
        0, sin(rotx), cos(rotx), 0,
        0, 0, 0, 1;
    float roty = -asin(axis[0]);
    translate2 << cos(roty), 0, sin(roty), 0,
        0, 1, 0, 0,
        -sin(roty), 0, cos(roty), 0,
        0, 0, 0, 1;
    trans = translate2 * translate1;

    Eigen::Matrix4f translate;
    float ang = angle / 180.0 * acos(-1);
    translate << cos(ang), -sin(ang), 0, 0,
        sin(ang), cos(ang), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    
    return trans.inverse() * translate * trans * rot;
}

// 直接调用Eigen库
Eigen::Matrix4f get_rotation1(const Eigen::Vector3f& axis, float angle) {
    // 归一化轴向量
    Eigen::Vector3f normalizedAxis = axis.normalized();

    // 创建围绕任意轴旋转的矩阵
    Eigen::AngleAxisf rotationAngle(Eigen::AngleAxisf(angle * MY_PI / 180.0, normalizedAxis));

    // 将AngleAxisf转换为旋转矩阵
    Eigen::Matrix4f rotationMatrix = Eigen::Matrix4f::Identity();
    rotationMatrix.block<3, 3>(0, 0) = rotationAngle.toRotationMatrix();

    return rotationMatrix;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    float rot = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    Eigen::Vector3f axis = { 1, 1, 1 };
    // 归一化轴向量
    axis = axis.normalized();

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_rotation(get_rotation(axis, rot));
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        //std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
        else if (key == 'q') {
            rot += 10;
        }
    }

    return 0;
}
