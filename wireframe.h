#ifndef WIREFRAME_H
#define WIREFRAME_H

#include <map>
#include "object.h"
#include "transformation.h"

using namespace std;

typedef struct perspective {
    double near;
    double far;
    double left;
    double right;
    double top;
    double bottom;
} perspective_t;

class Wireframe {
    public:
        /* File name used to populate Wireframe 
           from processFormatFile with '.txt' removed */
        string file_name;
        /* PPM Resolution */
        int xres, yres;
        /* Camera parameters */
        vertex_t cam_pos;
        vertex_t cam_orien;
        double cam_angle;
        /* Perspective parameters */
        perspective_t perspec;
        /* Transformation matrices*/
        Matrix4d cam_space_transform;
        Matrix4d perspec_proj_transform;
        /* Original read in objects mapped by name*/
        map<string, Object> objects;
        /* Copies of the read in objects that are
           transformed and mapped to a pixel grid */
        map<string, Object> copies;
        /* Cartesian NDC Pixel Grid 
           Each value [0 to 1] describes how much to shade in the pixel */
        float** grid;

        /** 
         * Populates Wireframe properties by reading from format .txt file.
         * 
         * @param filename of the .txt file to be processed
         * @throws invalid_argument if it fails to read the file
         */ 
        void processFormatFile(string filename);
        
        /**
         * Uses the camera and persepective parameters to compute
         * and save the camera and perspective transformation matrices.
        */
        void computeTransforms();

        /**
         * Applies the camera and persepective transformations to each object copy.
        */
        void applyTransforms();

        /**
         * Plots the tranformed object copies to the pixed grid.
         * 
         * This allocates data for grid, the only malloced attribute.
        */
        void plot(bool antialiase);

        /**
         * Writes the final output image computed as a PPM to a file.
         * Also prints the final image to standard out if printToStd is true.
         * 
         * @param printToStd boolean that decides if image is also printed
         * @throws invalid_argument if it fails to open the file
        */
        void output(bool printToStd);

        /**
         * Destructs the object by freeing any allocated data (grid)
        */
        void destruct();

    private:
        /**
         * Returns if the point (y,x) lies within the Pixel Grid.
         * @param y, the y component of the point (y, x)
         * @param x, the x component of the point (y, x)
        */
        bool pointInBound(int y, int x);

        /**
         * Plots the given point (y, x) if it lies within the Pixel Grid.
         * @param y, the y component of the point (y, x)
         * @param x, the x component of the point (y, x)
         * @param shade, the shade factor of how much to shade in the point
        */
        void plotPoint(int y, int x, float shade);

        /** 
         * Uses generalized application of Bresenham's Line Algorithm 
         * to rasterize a line on the Pixel Grid between 2 vertexes.
         * 
         * Note the vertexes given may not lie on the Pixel Grid.
         * 
         * @param v1, the first vertex given
         * @param v2, the second vertex given
         * @param antialiase, if true, antialiases rendered line (extra credit)
         */ 
        void bresenhamRasterize(grid_vertex_t v1, grid_vertex_t v2, bool antialiase);
};

#endif