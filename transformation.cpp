#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <time.h>

#include "utils.h"

#include "transformation.h"

using Eigen::Vector3d;

Matrix4d translation(double tx, double ty, double tz) {
    Matrix4d m;
    m << 1, 0, 0, tx,
         0, 1, 0, ty,
         0, 0, 1, tz,
         0, 0, 0, 1;
    return m;
}

Matrix4d rotation(double rx, double ry, double rz, double rad) {
    Vector3d checkNormalized(rx, ry, rz);
    checkNormalized.normalize();
    rx = checkNormalized[0];
    ry = checkNormalized[1];
    rz = checkNormalized[2];
    
    Matrix4d m;
    m << (rx*rx+(1-rx*rx)*cos(rad)), (rx*ry*(1-cos(rad))-rz*sin(rad)),
                                  (rx*rz*(1-cos(rad))+ry*sin(rad)), 0,
         (ry*rx*(1-cos(rad))+rz*sin(rad)), (ry*ry+(1-ry*ry)*cos(rad)),
                                  (ry*rz*(1-cos(rad))-rx*sin(rad)), 0,
         (rz*rx*(1-cos(rad))-ry*sin(rad)), (rz*ry*(1-cos(rad))+rx*sin(rad)),
                                  (rz*rz+(1-rz*rz)*cos(rad)), 0,
         0, 0, 0, 1;
    return m;
}

Matrix4d scaling(double sx, double sy, double sz) {
    Matrix4d m;
    m << sx, 0, 0, 0,
         0, sy, 0, 0,
         0, 0, sz, 0,
         0, 0, 0, 1;
    return m;
}

bool isIdentity(Matrix4d m) {
    Matrix4d identity;
    identity << 1, 0, 0, 0,
         0, 1, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            if (!isClose(identity.coeff(i, j), m.coeff(i, j))) {
                return false;
            }
        }
    }
    return true;
}

Matrix4d computeGrandProduct(string filename) {
    Matrix4d result;

    if (filename.find(".txt") == -1) {
        throw invalid_argument("File " + filename + " needs to be a .txt file.");
    }

    string buffer;
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (file.fail()) {
        throw invalid_argument("Could not read txt file '" + filename + "'.");
    }

    Matrix4d product;
    bool first_run = true;
    while (getline(file, buffer)) {
        vector<string> element;
        splitBySpace(buffer, element);

        Matrix4d curr;
        if (element[0][0] == 't') {
            curr = translation(stod(element[1]), stod(element[2]), stod(element[3]));
        } else if (element[0][0] == 'r') {
            curr = rotation(stod(element[1]), stod(element[2]), stod(element[3]), 
                                                        stod(element[4]));;
        } else {
            curr = scaling(stod(element[1]), stod(element[2]), stod(element[3]));
        }

        if (first_run) {
            product = curr;
            first_run = false;
            continue;
        }

        product = curr * product;
    }

    file.close();
    return product;
}

/** 
 * Checks if the rotation function outputs the correct matrixes.
 * @returns 0 if it works correctly, 1 otherwise
 */
int checkRotationMatrix() {
    for (size_t i = 0; i < 15; i++) {
        Matrix4d rot = rotation(rand_num(0,30), rand_num(0,30), rand_num(0,30), rand() * 1.0 /rand());
        Matrix4d shouldIdentity = rot * rot.inverse();
        if (!shouldIdentity.isIdentity()) {
            return 1;
        }
    }
    return 0;
}