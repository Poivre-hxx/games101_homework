#include<cmath>
#include<Eigen>
#include<iostream>

int main(){

    Eigen::Vector3f v(2.0f, 1.0f, 1.0f);
    Eigen::Matrix3f i, j;
    float rot = 45.0 / 180.0 * acos(-1);
    i << cos(rot), -sin(rot), 0, sin(rot), cos(rot), 0, 0, 0, 1;
    j << 1, 0, 1, 0, 1, 2, 0, 0, 1;
    std::cout << j * i * v << std::endl;

    return 0;
}