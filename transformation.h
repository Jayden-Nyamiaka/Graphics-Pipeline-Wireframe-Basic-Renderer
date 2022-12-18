#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H
 
#include <Eigen/Dense>
#include <string>
#include <stdexcept>

using Eigen::Matrix4d;

using namespace std;

Matrix4d translation(double tx, double ty, double tz);

/** 
 * Rotation about vector v = (rx, ry, rz) 
 * counterclockwise by angle rad in radians
 * 
 * Normalizes the given vector if it's not already a unit vector
  */
Matrix4d rotation(double rx, double ry, double rz, double rad);

Matrix4d scaling(double sx, double sy, double sz);

/**
 * Computes the product of all transformations from 
 * a given .obj file in the form of a 4D Matrix.
 * 
 * @throws invalid_argument if the file cannot be read
 * @return the 4D matrix result
 */
Matrix4d computeGrandProduct(string filename);

#endif