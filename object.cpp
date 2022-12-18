#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>

#include "utils.h"

#include "object.h"

vertex_t initVertex(double a, double b, double c) {
    vertex_t v;
    v.x = a;
    v.y = b;
    v.z = c;
    return v;
}

face_t initFace(int a, int b, int c) {
    face_t f;
    f.v1 = a;
    f.v2 = b;
    f.v3 = c;
    return f;
}

grid_vertex_t initGridVertex(int a, int b) {
    grid_vertex_t v;
    v.x = a;
    v.y = b;
    return v;
}

void Object::init() {
    vertexes.push_back(initVertex(0, 0, 0));
    pixels.push_back(initGridVertex(0, 0));
}

Object::Object() {
    init();
}

Object::Object(string filename) {
    init();
    processFile(filename);
}

Object Object::copy() {
    Object copy;
    copy.name = name;
    copy.vertexes = vertexes;
    copy.faces = faces;
    return copy;
}

void Object::processFile(string filename) {
    if (filename.find(".obj") == -1) {
        throw invalid_argument("File " + filename + " needs to be a .obj file.");
    }
    
    string buffer;
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (file.fail()) {
        string msg = "Could not read obj file '" + filename + "'.";
        throw invalid_argument(msg);
    }

    if (name.size() == 0) {
        filename.erase(filename.find('.'));
        name = filename;
    }
    
    vector<string> element;
    while (getline(file, buffer)) {
        element.clear();
        splitBySpace(buffer, element);
        if (element[0][0] == 'v') {
            vertex_t v = initVertex(stod(element[1]), stod(element[2]), stod(element[3]));
            vertexes.push_back(v);
        } else {
            face_t f = initFace(stoi(element[1]), stoi(element[2]), stoi(element[3]));
            faces.push_back(f);
        }

    }

    file.close();
}

void Object::printContents() {
    if (name.empty()) {
        cout << "Unnamed Object.\n";
    } else {
        cout << name << ":\n";
    }

    for (size_t i = 1; i < vertexes.size(); i++) {
        vertex_t v = vertexes[i];
        cout << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (size_t i = 0; i < faces.size(); i++) {
        face_t f = faces[i];
        cout << "f " << f.v1 << " " << f.v2 << " " << f.v3 << "\n";
    }

    cout << endl;
}

void Object::printPixels() {
    if (name.empty()) {
        cout << "Unnamed Object.\n";
    } else {
        cout << name << ":\n";
    }

    for (size_t i = 0; i < pixels.size(); i++) {
        cout << "(" << pixels[i].x << ", " << pixels[i].y << ")" << endl;
    }

    cout << endl;
}