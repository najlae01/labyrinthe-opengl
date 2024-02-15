#include "Mesh.h"
#include "ObjWGroupsLoader.h"

/*
    Loads OBJ file, parses the data, and returns a vector of Mesh objects
    Each Mesh represents a group in the OBJ file
*/
void ObjWGroupsLoader::loadObj(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier: " << filename << std::endl;
        return;
    }

    std::string line;
    Mesh currentMesh;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "o") {
            if (!currentMesh.name.empty()) {
                Meshes.push_back(currentMesh);
            }
            currentMesh = Mesh(); // Reset currentMesh
            currentMesh.facesValues.clear();
            currentMesh.name = getNextToken(iss);
            currentMesh.data.push_back("o " + currentMesh.name);
            currentMesh.hasFirstFaceValue = false;
        }
        else if (token == "v") {
            GLfloat x, y, z;
            iss >> x >> y >> z;
            currentMesh.data.push_back("v " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
        }
        else if (token == "f") {
            int indices[3];
            iss >> indices[0] >> indices[1] >> indices[2];
            for (int i = 0; i < 3; i++) {
                currentMesh.facesValues.push_back(indices[i]);
            }
            if (!currentMesh.hasFirstFaceValue) {
                currentMesh.firstFaceValue = indices[0];
                currentMesh.hasFirstFaceValue = true;
            }
        }
        else if (token == "s") {
            int smoothingGroup;
            iss >> smoothingGroup;
            currentMesh.data.push_back("s " + std::to_string(smoothingGroup));
        }
    }

    if (!currentMesh.name.empty()) {
        Meshes.push_back(currentMesh);
    }

    StoreFacesValuesInData();
}

void ObjWGroupsLoader::StoreFacesValuesInData() {
    for (int i = 0; i < Meshes.size(); i++) {
        Mesh& mesh = Meshes[i]; // Reference to avoid accessing Meshes[i] multiple times
        if (!mesh.facesValues.empty()) {
            mesh.firstFaceValue = mesh.facesValues[0];
            for (int j = 0; j < mesh.facesValues.size(); j++) {
                if (mesh.facesValues[j] < mesh.firstFaceValue) {
                    mesh.firstFaceValue = mesh.facesValues[j];
                }
            }
            //std::cout << "f: " << mesh.firstFaceValue << std::endl;

            for (int j = 0; j < mesh.facesValues.size(); j++) {
                mesh.facesValues[j] -= mesh.firstFaceValue;
                mesh.facesValues[j]++;
            }

            for (int j = 0; j < mesh.facesValues.size(); j += 3) {
                mesh.data.push_back("f " +
                    std::to_string(mesh.facesValues[j]) + " " +
                    std::to_string(mesh.facesValues[j + 1]) + " " +
                    std::to_string(mesh.facesValues[j + 2]));
            }
        }
    }
}


void ObjWGroupsLoader::printMeshFaces(Mesh currentMesh) {
    for (int j = 0; j < currentMesh.facesValues.size(); j++) {
        std::cout << "Face : " << currentMesh.facesValues[j] << std::endl;
    }
}


/*
    Displays the specified mesh using the provided render mode
    Assumes the mesh data contains vertex and face information
    Renders triangles for simplicity
*/
void ObjWGroupsLoader::displayMesh(const Mesh& mesh, GLenum renderMode) {

    // Create and bind VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind VBO for vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Extract vertices from mesh data
    std::vector<GLfloat> vertexData;
    for (const auto& line : mesh.data) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            GLfloat x, y, z;
            iss >> x >> y >> z;
            vertexData.push_back(x);
            vertexData.push_back(y);
            vertexData.push_back(z);
        }
    }

    // Upload vertex data to VBO
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    // Specify vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set render mode
    glPolygonMode(GL_FRONT_AND_BACK, renderMode);

    // Bind VAO
    glBindVertexArray(vao);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 3);

    // Unbind VAO
    glBindVertexArray(0);

    // Delete VBO and VAO
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

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