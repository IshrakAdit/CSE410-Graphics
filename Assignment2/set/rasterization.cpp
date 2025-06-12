#include <fstream>
#include <iomanip>

#include "transformations.cpp"
#include "bitmap_image.cpp"

using namespace std;

bool is_equal(double a, double b)
{
    return fabs(a - b) <= numeric_limits<double>::epsilon();
}

pair<bool, Point> check_line_segment_intersection(const Line &line,
                                                  Line segment)
{
    pair<bool, Point> intersection_point = line.get_intersection_point(segment);

    // No intersection
    if (!intersection_point.first)
        return intersection_point;

    // Checking if the line segment is vertical
    if (is_equal(segment.p0.x, segment.p1.x))
        // If yes, comparing if intersection point is bound by y-coordinates of line segment
        intersection_point.first = intersection_point.second.y >= min(segment.p0.y, segment.p1.y) && intersection_point.second.y <= max(segment.p0.y, segment.p1.y);
    else
        // Otherwise, comparing if intersection point is bound by y-coordinates of line segment
        intersection_point.first = intersection_point.second.x >= min(segment.p0.x, segment.p1.x) && intersection_point.second.x <= max(segment.p0.x, segment.p1.x);

    return intersection_point;
}

void rasterization(vector<Triangle> &triangles)
{

    // Input streams
    ifstream config_stream("config.txt");

    // Output streams
    ofstream z_buffer_stream("z_buffer.txt");
    z_buffer_stream << fixed << setprecision(6);

    // Sub-task-1: Read & Extract Data
    double screen_width, screen_height;
    config_stream >> screen_width >> screen_height;

    // Horizontal limits of the view space
    double left_limit, right_limit;
    config_stream >> left_limit;
    right_limit = (-1) * left_limit;

    // Vertical limits of the view space
    double bottom_limit, top_limit;
    config_stream >> bottom_limit;
    top_limit = (-1) * bottom_limit;

    // Depth range
    double z_min, z_max;
    config_stream >> z_min >> z_max;

    for (Triangle &triangle : triangles)
        triangle.set_random_colors();

    // Size of a pixel in normalized coordinates
    double pixel_width = (right_limit - left_limit) / screen_width;
    double pixel_height = (top_limit - bottom_limit) / screen_height;

    // Center coordinates of edge pixels
    double topmost_center_y = top_limit - pixel_height / 2.0;
    double bottommost_center_y = bottom_limit + pixel_height / 2.0;
    double leftmost_center_x = left_limit + pixel_width / 2.0;
    double rightmost_center_x = right_limit - pixel_width / 2.0;

    // Sub-task-2: Initialize Z-buffer and Frame buffer
    vector<vector<double>> z_buffer(screen_height, vector<double>(screen_width, z_max));
    bitmap_image image(screen_width, screen_height);
    image.set_all_channels(0, 0, 0);

    // Sub-task-3: Apply procedure
    for (int tr = 0; tr < triangles.size(); tr++)
    {
        Triangle triangle = triangles[tr];
        triangle.reorder_vertices();

        // Trianle vertices -> A, B, C
        Vector A = Vector(triangle.vertices[0].elements[0][0], triangle.vertices[0].elements[1][0], triangle.vertices[0].elements[2][0]);
        Vector B = Vector(triangle.vertices[1].elements[0][0], triangle.vertices[1].elements[1][0], triangle.vertices[1].elements[2][0]);
        Vector C = Vector(triangle.vertices[2].elements[0][0], triangle.vertices[2].elements[1][0], triangle.vertices[2].elements[2][0]);

        // Projection of triangle edges on xy plane ( z = 0 )
        Line projection_of_AB = Line(Point(A.x, A.y, 0), Point(B.x, B.y, 0));
        Line projection_of_AC = Line(Point(A.x, A.y, 0), Point(C.x, C.y, 0));
        Line projection_of_BC = Line(Point(B.x, B.y, 0), Point(C.x, C.y, 0));

        double bottom_scanline = max(C.y, bottommost_center_y);
        double top_scanline = min(A.y, topmost_center_y);

        int top_row = round((top_scanline - bottommost_center_y) / pixel_height);
        int bottom_row = round((bottom_scanline - bottommost_center_y) / pixel_height);

        for (int i = top_row; i >= bottom_row; i--)
        {
            double y_s = bottommost_center_y + i * pixel_height;

            Line current_line = Line(Point(0, y_s, 0), Point(1, y_s, 0));

            pair<bool, Point> AB_line_intersection = check_line_segment_intersection(current_line, projection_of_AB);
            pair<bool, Point> AC_line_intersection = check_line_segment_intersection(current_line, projection_of_AC);
            pair<bool, Point> BC_line_intersection = check_line_segment_intersection(current_line, projection_of_BC);

            int intersection_count = AB_line_intersection.first + AC_line_intersection.first + BC_line_intersection.first;

            auto reorder_points_and_lines = [&](string order)
            {
                if (order[1] > order[2])
                    swap(order[1], order[2]);
                if (order == "ABC")
                {
                    // No change
                }
                else if (order == "BAC")
                {
                    tie(A, B, C) = make_tuple(B, A, C);
                    tie(projection_of_AB, projection_of_AC, projection_of_BC) =
                        make_tuple(projection_of_AB, projection_of_BC, projection_of_AC);
                    tie(AB_line_intersection, AC_line_intersection, BC_line_intersection) =
                        make_tuple(AB_line_intersection, BC_line_intersection, AC_line_intersection);
                }
                else if (order == "CAB")
                {
                    tie(A, B, C) = make_tuple(C, A, B);
                    tie(projection_of_AB, projection_of_AC, projection_of_BC) =
                        make_tuple(projection_of_AC, projection_of_BC, projection_of_AB);
                    tie(AB_line_intersection, AC_line_intersection, BC_line_intersection) =
                        make_tuple(AC_line_intersection, BC_line_intersection, AB_line_intersection);
                }
            };

            if (intersection_count == 0)
                continue;
            if (intersection_count == 2)
            {
                reorder_points_and_lines(AB_line_intersection.first
                                             ? (AC_line_intersection.first ? "ABC" : "BAC")
                                             : "CAB");
            }

            double x_a = AB_line_intersection.second.x;
            double x_b = AC_line_intersection.second.x;
            double z_a = A.z - (A.z - B.z) * (A.y - y_s) / (A.y - B.y);
            double z_b = A.z - (A.z - C.z) * (A.y - y_s) / (A.y - C.y);

            if (x_a > x_b)
            {
                swap(x_a, x_b);
                swap(z_a, z_b);
            }

            int left_col = round((max(x_a, leftmost_center_x) - leftmost_center_x) / pixel_width);
            int right_col = round((min(x_b, rightmost_center_x) - leftmost_center_x) / pixel_width);

            for (int j = left_col; j <= right_col; j++)
            {
                double x_p = leftmost_center_x + j * pixel_width;
                double z_p = z_b - (z_b - z_a) * (x_b - x_p) / (x_b - x_a);

                if (z_p >= z_min && z_p < z_buffer[screen_height - 1 - i][j])
                {
                    z_buffer[screen_height - 1 - i][j] = z_p;
                    image.set_pixel(j, screen_height - 1 - i, triangle.red,
                                    triangle.green, triangle.blue);
                }
            }
        }
    }

    // Sub-task-4: Save image and z_buffer
    image.save_image("out.bmp");

    for (int i = 0; i < screen_height; i++)
    {
        for (int j = 0; j < screen_width; j++)
        {
            if (z_buffer[i][j] >= z_max)
                continue;
            z_buffer_stream << z_buffer[i][j] << "\t";
        }
        z_buffer_stream << endl;
    }

    // Sub-task-5: Free all memory
    for (auto &row : z_buffer)
        row.clear();
    z_buffer.clear();

    image.clear();

    // All file streams closed
    config_stream.close();
    z_buffer_stream.close();
}