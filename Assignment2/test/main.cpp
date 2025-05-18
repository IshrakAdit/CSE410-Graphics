#include <iostream>
#include <fstream>
#include <stack>

#include "triangle.cpp"

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

    stack<Matrix> s;
    s.push(generateIdentityMatrix(4));

    stack<Triangle> triangles;

    string command;
    while (true)
    {
        scene_stream.ignore(256, '\n');
        scene_stream >> command;
        cout << command << endl;

        if (command == "triangle")
        {
            Triangle triangle;
            scene_stream >> triangle;
            triangle.transform(s.top());
            triangles.push(triangle);
            stage1_stream << triangle << endl;
            stage1_stream << endl;
        }
        else if (command == "end")
        {
            break;
        }
        // else
        // {
        //     cerr << "Invalid command: " << command << endl;
        //     return -1;
        // }
    }

    // All file streams closed
    scene_stream.close();
    stage1_stream.close();
    stage2_stream.close();
    stage3_stream.close();
}