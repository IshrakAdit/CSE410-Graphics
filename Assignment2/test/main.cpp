#include <iostream>
#include <fstream>
#include "vector.cpp"

using namespace std;

int main(void)
{
    // Input streams
    ifstream scene_stream("scene.txt");

    // Output streams
    ofstream stage1_stream("stage1.txt");
    ofstream stage2_stream("stage2.txt");
    ofstream stage3_stream("stage3.txt");

    // Input parameters
    Vector eye, look, up;
    double fovY, aspectRatio, near, far;

    // Camera params from scene file
    scene_stream >> eye >> look >> up >> fovY >> aspectRatio >> near >> far;

    cout << eye;
    cout << look;
    cout << up;

    // All file streams closed
    scene_stream.close();
    stage1_stream.close();
    stage2_stream.close();
    stage3_stream.close();
}