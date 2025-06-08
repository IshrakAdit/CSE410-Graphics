#include <fstream>
#include <stack>
#include <iomanip>

#include "transformations.cpp"
#include "bitmap_image.cpp"

using namespace std;

int main(void)
{
    // Input streams
    ifstream scene_stream("scene.txt");
    ifstream config_stream("config.txt");

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
        cout << tx_command << endl;

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

    double screen_width, screen_height;
    config_stream >> screen_width >> screen_height;

    for (Triangle &triangle : triangles)
        triangle.set_random_colors();

    // Depth range
    double z_min = -1.0, z_max = 1.0;

    // Horizontal and Vertical limits of the view space
    double left_limit = -1.0, right_limit = 1.0;
    double bottom_limit = -1.0, top_limit = 1.0;

    // Size of a pixel in normalized coordinates
    double pixel_width = (right_limit - left_limit) / screen_width;
    double pixel_height = (top_limit - bottom_limit) / screen_height;

    // Center coordinates of edge pixels
    double topmost_center_y = top_limit - pixel_height / 2.0;
    double bottommost_center_y = bottom_limit + pixel_height / 2.0;
    double leftmost_center_x = left_limit + pixel_width / 2.0;
    double rightmost_center_x = right_limit - pixel_width / 2.0;

    vector<vector<double>> z_buffer(screen_height, std::vector<double>(screen_width, z_max));
    bitmap_image image(screen_width, screen_height);
    image.set_all_channels(0, 0, 0);

    // All file streams closed
    scene_stream.close();
    stage1_stream.close();
    stage2_stream.close();
    stage3_stream.close();
}