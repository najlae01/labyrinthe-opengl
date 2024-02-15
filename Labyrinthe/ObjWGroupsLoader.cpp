#include "Mesh.h"
#include "ObjWGroupsLoader.h"

/*
    Loads OBJ file, parses the data, and returns a vector of Mesh objects
    Each Mesh represents a group in the OBJ file
*/
std::vector<Mesh> ObjWGroupsLoader::loadObj(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier: " << filename << std::endl;
        return {};
    }

    std::vector<Mesh> meshes;
    Mesh currentMesh;

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "o") {
            // Start of a new group (mesh)
            if (!currentMesh.name.empty()) {
                meshes.push_back(currentMesh);
            }
            currentMesh.name = getNextToken(iss);
            currentMesh.data.clear();
            currentMesh.data.push_back("g " + currentMesh.name);
            currentMesh.hasFirstFaceValue = false; // Resetting flag for each mesh
            currentMesh.firstFaceValue = std::numeric_limits<int>::max(); // Initialize with maximum possible value
        }

        // Storing the line data for the current group
        if (!currentMesh.name.empty()) {
            if (token == "v") {
                // Vertex coordinates
                GLfloat x, y, z;
                iss >> x >> y >> z;
                currentMesh.data.push_back("v " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
            }
            else if (token == "f") {
                std::vector<int> indices;
                int index;
                while (iss >> index) {
                    indices.push_back(index);
                    // Ignore the remaining indices
                    iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                }
                if (!indices.empty()) {
                    // Update min face value
                    for (int index : indices) {
                        if (index < currentMesh.firstFaceValue) {
                            currentMesh.firstFaceValue = index;
                        }
                    }
                    currentMesh.hasFirstFaceValue = true;

                    // Adjust face values and push to current mesh data
                    std::ostringstream oss;
                    for (int index : indices) {
                        oss << (index - currentMesh.firstFaceValue + 1) << " ";
                    }
                    currentMesh.data.push_back("f " + oss.str());
                    //std::cout << "f: " << indices[0] << ", " << indices[1] << ", " << indices[2] << std::endl;

                }
            }
            else if (token == "s") {
                int smoothingGroup;
                iss >> smoothingGroup;
                currentMesh.data.push_back("s " + std::to_string(smoothingGroup));

            }
        }
    }

    if (!currentMesh.name.empty()) {
        meshes.push_back(currentMesh);
    }

    return meshes;
}


/*
    Displays the specified mesh using the provided render mode
    Assumes the mesh data contains vertex and face information
    Renders triangles for simplicity
*/
void ObjWGroupsLoader::displayMesh(const Mesh& mesh, GLenum renderMode) {

    std::vector<glm::vec3> vertices;

    // Set render mode
    glPolygonMode(GL_FRONT_AND_BACK, renderMode);

    glBegin(GL_TRIANGLES);

    for (const auto& line : mesh.data) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            // Vertex coordinates
            GLfloat x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(glm::vec3(x, y, z));
        }
        else if (token == "f") {
            // Face definition (assuming triangles for simplicity)
            int v1, v2, v3;
            iss >> v1 >> v2 >> v3;

            // Assuming vertex indices start from 1 in OBJ files
            // Adjust if indices start from 0
            v1--; v2--; v3--;

            // Assuming vertices are specified in counter-clockwise order
            glVertex3f(vertices[v1].x, vertices[v1].y, vertices[v1].z);
            glVertex3f(vertices[v2].x, vertices[v2].y, vertices[v2].z);
            glVertex3f(vertices[v3].x, vertices[v3].y, vertices[v3].z);
        }
    }

    glEnd();

    // Reset render mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/*
    Retrieves the next token from the istringstream
    Used for parsing OBJ file data
*/
std::string ObjWGroupsLoader::getNextToken(std::istringstream& iss) {
    std::string token;
    iss >> token;
    return token;
}


std::vector<std::string> ObjWGroupsLoader::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}