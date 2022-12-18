#ifndef OBJECT_H
#define OBJECT_H
 
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

typedef struct vertex {
    double x;
    double y;
    double z;
} vertex_t;

vertex_t initVertex(double a, double b, double c);

typedef struct face {
    int v1;
    int v2;
    int v3;
} face_t;

face_t initFace(int a, int b, int c);

typedef struct gridVertex {
    int x;
    int y;
} grid_vertex_t;

grid_vertex_t initGridVertex(int a, int b);

class Object {
    public:
        string name;
        vector<vertex_t> vertexes;
        vector<face_t> faces;

        /* All of Object's vertexes that are inside the 
           perspective cube mapped to a 2D NDC pixel grid */
        vector<grid_vertex_t> pixels;

        /**
         * Base constructor that simply initializes vertexes and faces.
         */
        Object();

        /**
         * Constructor that populates Object with vertexes 
         * and faces from the 'filename' .obj file.
         * 
         * @param filename of the .obj file to be processed
         * @throws invalid_argument if it fails to read the file
         */
        Object(string filename);

        /**
         * Returns a deep copy of the object.
         * 
         * @returns a deep copy of Object
         */
        Object copy();

        /** 
         * Populates Object with the vertexes and faces
         * retrieved from reading the 'filename' .obj file.
         * 
         * If no name, names the object the filename 
         * 
         * @param filename of the .obj file to be processed
         * @throws invalid_argument if it fails to read the file
         */ 
        void processFile(string filename);

        /**
         * Prints out all the vertexes and faces.
         */
        void printContents(); 

        /**
         * Prints out all the Pixel Grid vertexes.
         */
        void printPixels();

    private:
        /* Private helper functions the user doesn't need to know */
        void init();
        void setFileName(string filename);
};

#endif