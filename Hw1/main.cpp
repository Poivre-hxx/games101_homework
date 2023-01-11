#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

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

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    Eigen::Matrix4f rotate;
    rotate << std::cos(rotation_angle), std::sin(-rotation_angle), 0, 1,
        std::sin(rotation_angle), std::cos(rotation_angle), 0, 1,
        0, 0, 1, 0,
        0, 0, 0, 1;

    model = rotate * model;
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f trans1 = Eigen::Matrix4f::Identity();
    trans1 << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;
    
    Eigen::Matrix4f trans2 = Eigen::Matrix4f::Identity();
    trans2 << 1 / (zNear * std::tan(eye_fov / 2) * aspect_ratio), 0, 0, 0,
        0, 1 / (zNear * std::tan(eye_fov / 2)), 0, 0,
        0, 0, -2 / (zFar - zNear), -(zNear + zFar) / (zFar - zNear), 
        0, 0, 0, 1; 
    
    projection = trans2 * trans1 * projection;
    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rotate1 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f model1 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f rotate2 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f rot1 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f rot2 = Eigen::Matrix4f::Identity();

    model1 = get_model_matrix(angle);

    float ang1 = 2 * std::atan(axis[0] / 2/ axis[1]);
    float ang2 = std::atan(axis[1] / axis[2]);
    
    rotate1 << std::cos(ang1), std::sin(-ang1), 0, 0,
        std::sin(ang1), std::cos(ang1), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1; 
    rotate2 << std::cos(ang2), std::sin(-ang2), 0, 0,
        std::sin(ang2), std::cos(ang2), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1; 
    rot1 = rotate2 * rotate1;

    ang1 = -2 * std::atan(axis[0] / 2/ axis[1]);
    ang2 = -1 * std::atan(axis[1] / axis[2]);
    rotate1 << std::cos(ang1), std::sin(-ang1), 0, 0,
        std::sin(ang1), std::cos(ang1), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1; 
    rotate2 << std::cos(ang2), std::sin(-ang2), 0, 0,
        std::sin(ang2), std::cos(ang2), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1; 
    rot2 = rotate1 * rotate2;

    model = rot2 * model1 * rot1;
}

int main(int argc, const char** argv)
{
    float angle = 0;
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

    Eigen::Vector3f axis = {1, 1, 1};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_rotation(axis, angle));
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

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
