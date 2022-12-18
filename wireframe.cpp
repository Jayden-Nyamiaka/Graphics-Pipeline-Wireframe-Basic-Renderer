#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>

#include "utils.h"
#include "transformation.h"
#include "color.h"
#include "wireframe.h"

using Eigen::Vector4d;


/* Helper method for Wireframe::processFormatFile */
void saveTransformedCopy(map<string, Object> &objects,
                         map<string, Object> &copies,
                         string objectName, 
                         Matrix4d transformation) {
    Object objCopy = objects[objectName].copy();

    /* Generates a unique name for the copy */
    int copyNumber = 1;
    string nameAttempt = objectName + "_copy" + to_string(copyNumber);
    while (copies.find(nameAttempt) != copies.end()) {
        copyNumber++;
        nameAttempt = objectName + "_copy" + to_string(copyNumber);
    }
    objCopy.name = nameAttempt;

    /* Transforms points of copy via the instructions of the format file */
    for (size_t i = 1; i < objCopy.vertexes.size(); i++) {
        Vector4d point(objCopy.vertexes[i].x, objCopy.vertexes[i].y, objCopy.vertexes[i].z, 1);
        Vector4d result = transformation * point;
        objCopy.vertexes[i].x = result[0] / result[3];
        objCopy.vertexes[i].y = result[1] / result[3];
        objCopy.vertexes[i].z = result[2] / result[3];
    }

    copies.insert({objCopy.name, objCopy});
}


void Wireframe::processFormatFile(string filename) {
    if (filename.find(".txt") == -1) {
        throw invalid_argument("File " + filename + " needs to be a .txt file.");
    }

    string buffer;
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (file.fail()) {
        throw invalid_argument("Could not read format file '" + filename + "'.");
    }

    /* Saves filename to Wireframe except w/o the '.txt' suffix */
    filename.erase(filename.find('.'));
    file_name = filename;

    vector<string> line;

    /* Reads in camera and perspective parameters */
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (line[0] == "objects:") {
            break;
        } else if (line[0] == "position") {
            cam_pos = initVertex(stod(line[1]), stod(line[2]), stod(line[3]));
        } else if (line[0] == "orientation") {
            cam_orien = initVertex(stod(line[1]), stod(line[2]), stod(line[3]));
            cam_angle = stod(line[4]);
        } else if (line[0] == "near") {
            perspec.near = stod(line[1]);
        } else if (line[0] == "far") {
            perspec.far = stod(line[1]);
        } else if (line[0] == "left") {
            perspec.left = stod(line[1]);
        } else if (line[0] == "right") {
            perspec.right = stod(line[1]);
        } else if (line[0] == "top") {
            perspec.top = stod(line[1]);
        } else if (line[0] == "bottom") {
            perspec.bottom = stod(line[1]);
        }
    }

    /* Reads in all objects storing them in object maps */
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (line.size() == 0) {
            break;
        }

        Object obj = Object("data/" + line[1]);
        obj.name = line[0];
        objects.insert({obj.name, obj});
    }

    /* Reads in all tranformations, making copies of objects + applying tranformations */
    string objectName = "";
    Matrix4d transformation;
    bool first_run = true;
    while (getline(file, buffer)) {
        line.clear();
        splitBySpace(buffer, line);

        if (objectName.empty()) {
            objectName = line[0];
            continue;
        }

        if (line.size() == 0) {
            saveTransformedCopy(objects, copies, objectName, transformation);
            objectName = "";
            first_run = true;
            continue;
        }

        /* Processes one line of the file as a transformation matrix */
        Matrix4d curr;
        if (line[0][0] == 't') {
            curr = translation(stod(line[1]), stod(line[2]), stod(line[3]));
        } else if (line[0][0] == 'r') {
            curr = rotation(stod(line[1]), stod(line[2]), stod(line[3]), 
                                                        stod(line[4]));;
        } else {
            curr = scaling(stod(line[1]), stod(line[2]), stod(line[3]));
        }

        if (first_run) {
            transformation = curr;
            first_run = false;
            continue;
        }

        transformation = curr * transformation;
    }
    saveTransformedCopy(objects, copies, objectName, transformation);
    file.close();
}


void Wireframe::computeTransforms() {
    Matrix4d m_TcTr = translation(cam_pos.x, cam_pos.y, cam_pos.z) *
                 rotation(cam_orien.x, cam_orien.y, cam_orien.z, cam_angle);
    cam_space_transform = m_TcTr.inverse();

    double div_r1 = perspec.right - perspec.left;
    double div_r2 = perspec.top - perspec.bottom;
    double div_r3 = perspec.far - perspec.near;
    perspec_proj_transform << 
        (2 * perspec.near) / div_r1, 0, (perspec.right + perspec.left) / div_r1, 0,
        0, (2 * perspec.near) / div_r2, (perspec.top + perspec.bottom) / div_r2, 0,
        0, 0, -(perspec.far + perspec.near) / div_r3, -2 * perspec.far * perspec.near / div_r3,
        0, 0, -1, 0;
    
    return;
}


void Wireframe::applyTransforms() {
    Matrix4d homogenousNDC_transform = perspec_proj_transform * cam_space_transform;
    for (map<string, Object>::iterator iter = copies.begin(); 
                                    iter != copies.end(); iter++) {
        Object& copy = copies[iter->first];

        for (size_t i = 1; i < copy.vertexes.size(); i++) {
            Vector4d point(copy.vertexes[i].x, copy.vertexes[i].y, copy.vertexes[i].z, 1);
            Vector4d result = homogenousNDC_transform * point;
            
            copy.vertexes[i].x = result[0] / result[3];
            copy.vertexes[i].y = result[1] / result[3];
            copy.vertexes[i].z = result[2] / result[3];

            /* 
             * Mapping: [x, y] --> 
             * [(x + left) * xres / (right + left), (top - y) * yres / (top + bottom)]
            */
            int grid_x = round(0.5 * xres * ((copy.vertexes[i].x - perspec.left) / 
                    (perspec.right - perspec.left) + 0.5) );

            int grid_y = round(0.5 * yres * ((perspec.top - copy.vertexes[i].y) / 
                    (perspec.top - perspec.bottom) + 0.5) );

            copy.pixels.push_back(initGridVertex(grid_x, grid_y));
        }
        
    }
}


bool Wireframe::pointInBound(int y, int x) {
    if (y < 0 || y >= yres || x < 0 || x >= xres) {
        return false;
    }
    return true;
}


void Wireframe::plotPoint(int y, int x, float shade) {
    if (pointInBound(y, x)) {
        grid[y][x] = shade;
    }
}


void Wireframe::bresenhamRasterize(grid_vertex_t v1, grid_vertex_t v2, bool antialiase) {
    if (!pointInBound(v1.y, v1.x) || !pointInBound(v2.y, v2.x)) {
        return;
    }

    // Ensures lower vertex to upper vertex has ascending x-axis order
    grid_vertex_t lower, upper;
    if (v1.x < v2.x) {
        lower = v1;
        upper = v2;
    } else {
        lower = v2;
        upper = v1;
    }

    // Calculates slope, dx, dy
    int dx = upper.x - lower.x;
    int dy = upper.y - lower.y;
    float slope = dy * 1.0 / dx;

    /* 
     Flags if slope is negative
     If slope is negative, reflects upper.y over the line y = lower.y
     For negative slopes, we'll reflect upper.y to treat it as the equivalent
     positive slope case, then reflect the point back when its time to plot.
    */
    bool negative_slope = false;
    if (slope < 0) {
        negative_slope = true;
        upper.y = 2 * lower.y - upper.y;
        dy *= -1;
        slope *= -1;
    }

    /* 
     base is the axis that changes less and increases conditionally.
     increment is the axis we iterate over and increases every iteration.
     Defaults base & increment to iterate over x i.e mild slopes (m: [-1, 1])
     If steep slope (|m| > 1), sets base & increment to iterate over y
    */
    int base = lower.y, d_base = dy;
    int incr_low = lower.x, incr_up = upper.x, d_incr = dx;
    bool iterate_over_x = true;
    if (slope < -1 || slope > 1) {
        base = lower.x;
        d_base = dx;
        incr_low = lower.y;
        incr_up = upper.y;
        d_incr = dy;
        iterate_over_x = false;
    }

    int eps_d = 0;
    for (int incr = incr_low; incr <= incr_up; incr++) {

        // checks if we're iterating over x or y
        if (iterate_over_x) {
            if (antialiase && incr != incr_low && incr != incr_up) {
                float float_base = base + slope;
                float intensity = float_base - base;
                if (negative_slope) {
                    plotPoint(2 * lower.y - base, incr, 1.0 - intensity);
                    plotPoint(2 * lower.y - base - 1, incr, intensity);
                } else {
                    plotPoint(base, incr, 1.0 - intensity);
                    plotPoint(base + 1, incr, intensity);
                }
            } else {
                if (negative_slope) {
                    plotPoint(2 * lower.y - base, incr, 1);
                } else {
                    plotPoint(base, incr, 1);
                }
            }
        } else {
            if (antialiase && incr != incr_low && incr != incr_up) {
                float float_base = base + 1.0 / slope;
                float intensity = float_base - base;
                if (negative_slope) {
                    plotPoint(2 * lower.y - incr, base, 1.0 - intensity);
                    plotPoint(2 * lower.y - incr, base + 1, intensity);
                } else {
                    plotPoint(incr, base, 1.0 - intensity);
                    plotPoint(incr, base + 1, intensity);
                }
            } else {
                if (negative_slope) {
                    plotPoint(2 * lower.y - incr, base, 1);
                } else {
                    plotPoint(incr, base, 1);
                }
            }
        }

        eps_d += d_base;

        // checks if slope is negative to use appropriate comparison & change
        if ((eps_d << 1) >= d_incr) {
            base++;
            eps_d -= d_incr;
        }
    }

    return;
}


void Wireframe::plot(bool antialiase) {
    // Allocates data for and zeroes out Pixel Grid
    grid = (float **) malloc(yres * sizeof(float *));
    for (int y = 0; y < yres; y++) {
        grid[y] = (float *) malloc(xres * sizeof(float));
        for (int x = 0; x < xres; x++) {
            grid[y][x] = 0;
        }
    }

    /* Renders all lines that lie on the Pixel Grid by computing Bresenham's 
       Algorithm for the 3 lines of every face of every copied object */
    for (map<string, Object>::iterator obj_iter = copies.begin(); 
                                    obj_iter != copies.end(); obj_iter++) {
        for (size_t face_idx = 0; 
                face_idx < copies[obj_iter->first].faces.size(); face_idx++) {
            face_t face = copies[obj_iter->first].faces[face_idx];
            grid_vertex_t v1 = copies[obj_iter->first].pixels[face.v1];
            grid_vertex_t v2 = copies[obj_iter->first].pixels[face.v2];
            grid_vertex_t v3 = copies[obj_iter->first].pixels[face.v3];
            bresenhamRasterize(v1, v2, antialiase);
            bresenhamRasterize(v2, v3, antialiase);
            bresenhamRasterize(v3, v1, antialiase);
        }
    }
}


void Wireframe::output(bool printToStd) {
    ofstream ppm;
    string filename = file_name + ".ppm";
    ppm.open(filename.c_str(), ios::out);
    if (!ppm) {
        string msg = "Could not create '" + filename + "'.";
        throw runtime_error(msg);
    }
    
    ppm << "P3\n" << xres << " " << yres << "\n255\n";
    if (printToStd) {
        cout << "P3\n" << xres << " " << yres << "\n255\n";
    }

    color_rgb_t color = initColor(255, 255, 255);
    string unfilledStr = "0 0 0";

    for (int y = 0; y < yres; y++) {
        for (int x = 0; x < xres; x++) {
            float fill = grid[y][x];
            if (fill == 0) {
                ppm << unfilledStr << endl;
                if (printToStd) {
                    cout << unfilledStr << endl;
                }
            } else {
                string magnitudeStr = toString(scaleColor(color, fill));
                ppm << magnitudeStr << endl;
                if (printToStd) {
                    cout << magnitudeStr << endl;
                }
            }
        }
    }

    ppm.close();
}


void Wireframe::destruct() {
    for (int y = 0; y < yres; y++) {
        free(grid[y]);
    }
    free(grid);
}


void usage(void) {
    cerr << "Enter input in the form: scene_description_file.txt xres yres\n\t"
            "xres, yres must be positive integers\n";
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        usage();
    }

    try {
        Wireframe pipeline;
        pipeline.xres = stoi(argv[2]);
        pipeline.yres = stoi(argv[3]);
        if (pipeline.xres <= 0 || pipeline.yres <= 0) {
            usage();
        }
        pipeline.processFormatFile(argv[1]);
        pipeline.computeTransforms();
        pipeline.applyTransforms();
        pipeline.plot(true);
        pipeline.output(true);
        pipeline.destruct();
    } catch (const char *msg) {
        cerr << msg << endl;
    }

    return 0;
}