#include <stack>
#include "rasterization.cpp"

using namespace std;

int main(void)
{
    // Input streams
    ifstream scene_stream("scene.txt");

    // Output streams
    ofstream stage1_stream("stage1.txt");
    ofstream stage2_stream("stage2.txt");
    ofstream stage3_stream("stage3.txt");

    // Output precisions
    stage1_stream << fixed << setprecision(7);
    stage2_stream << fixed << setprecision(7);
    stage3_stream << fixed << setprecision(7);

    // Input parameters
    Vector eye, look, up;
    double fovY, aspectRatio, near, far;

    // Camera params from scene file
    scene_stream >> eye >> look >> up >> fovY >> aspectRatio >> near >> far;

    stack<Matrix> s;
    s.push(generateIdentityMatrix(4));

    vector<Triangle> triangles;

    // Translation parameters
    double tx, ty, tz;
    // Scale parameters
    double sx, sy, sz;
    // Rotation parameters
    double angle, rx, ry, rz;

    // Modelling Transformation
    string tx_command;
    while (true)
    {
        scene_stream.ignore(256, '\n');
        scene_stream >> tx_command;

        if (tx_command == "triangle")
        {
            Triangle triangle;
            scene_stream >> triangle;
            triangle.transform(s.top());
            triangles.push_back(triangle);
            stage1_stream << triangle << endl;
            stage1_stream << endl;
        }
        else if (tx_command == "translate")
        {
            scene_stream >> tx >> ty >> tz;
            Matrix translation_matrix = translationMatrix(tx, ty, tz);
            s.top() = s.top() * translation_matrix;
        }
        else if (tx_command == "scale")
        {
            scene_stream >> sx >> sy >> sz;
            Matrix scaling_matrix = scalingMatrix(sx, sy, sz);
            s.top() = s.top() * scaling_matrix;
        }
        else if (tx_command == "rotate")
        {
            scene_stream >> angle >> rx >> ry >> rz;
            Matrix rotation_matrix = rotationMatrix(rx, ry, rz, angle);
            s.top() = s.top() * rotation_matrix;
        }
        else if (tx_command == "push")
        {
            s.push(s.top());
        }
        else if (tx_command == "pop")
        {
            s.pop();
        }
        else if (tx_command == "end")
        {
            break;
        }
        else
        {
            cerr << "Invalid command: " << tx_command << endl;
            return -1;
        }
    }

    // View Transformation
    Matrix view_matrix = viewMatrix(eye, look, up);
    for (Triangle &triangle : triangles)
    {
        triangle.transform(view_matrix);
        stage2_stream << triangle << endl;
        stage2_stream << endl;
    }

    // Projection Transformation
    Matrix projection_matrix = projectionMatrix(fovY, aspectRatio, near, far);
    for (Triangle &triangle : triangles)
    {
        triangle.transform(projection_matrix);
        stage3_stream << triangle << endl;
        stage3_stream << endl;
    }

    // Clippinng & Rasterization
    rasterization(triangles);

    // Free all memory
    triangles.clear();

    // All file streams closed
    scene_stream.close();

    stage1_stream.close();
    stage2_stream.close();
    stage3_stream.close();

    return 0;
}