/*
    The ObjLoader class is responsible for loading, saving, and displaying 3D mesh data
    from OBJ files. It works with a vector of Mesh objects to represent individual
    meshes or objects in a file or a scene
*/

#ifndef OBJWGROUPSLOADER_H_INCLUDED
#define OBJWGROUPSLOADER_H_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include "Mesh.h"


class ObjWGroupsLoader {
public:
    std::vector<Mesh> loadObj(std::string filename);
    void displayMesh(const Mesh& mesh, GLenum renderMode);


private:
    std::string getNextToken(std::istringstream& iss);
    static std::vector<std::string> split(const std::string& s, char delimiter);
};


#endif // OBJWGROUPSLOADER_H_INCLUDED
