#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <GL/glew.h>
#define GLEW_STATIC
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "ObjLoader.h"
#include "TextureLoader.h"
#include "Camera.h"
#include "GLDebugDrawer.h"
#include "OpenGLMotionState.h"
#include "Mesh.h"
#include "ObjWGroupsLoader.h"


GLuint WIDTH = 1280;
GLuint HEIGHT = 720;
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

bool left = false, right = false, forward = false, backward = false;

glm::vec3 agentPos(2.0f, -4.0f, -10.0f);
float agentSpeed = 0.015f;

Camera cam;
GLint projLoc;

GLuint VAO[4];
GLuint VBO[4];
GLuint textures[4];

// Function to calculate the bounding box of a mesh
std::pair<glm::vec3, glm::vec3> calculateBoundingBox(const Mesh& mesh) {
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    // Iterate over mesh data to find vertices
    for (const std::string& line : mesh.data) {
        // Process line to find vertices
        if (line.substr(0, 2) == "v ") {
            float x, y, z;
            sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z);

            // Update minBounds and maxBounds based on vertex coordinates
            minBounds.x = std::min(minBounds.x, x);
            minBounds.y = std::min(minBounds.y, y);
            minBounds.z = std::min(minBounds.z, z);

            maxBounds.x = std::max(maxBounds.x, x);
            maxBounds.y = std::max(maxBounds.y, y);
            maxBounds.z = std::max(maxBounds.z, z);
        }
    }

    return { minBounds, maxBounds };
}

// Function to check for collision between two bounding boxes
bool checkCollision(const glm::vec3& agentMinBounds, const glm::vec3& agentMaxBounds,
    const glm::vec3& wallMinBounds, const glm::vec3& wallMaxBounds) {
    // Check for intersection between bounding boxes
    if (agentMaxBounds.x >= wallMinBounds.x && agentMinBounds.x <= wallMaxBounds.x &&
        agentMaxBounds.y >= wallMinBounds.y && agentMinBounds.y <= wallMaxBounds.y &&
        agentMaxBounds.z >= wallMinBounds.z && agentMinBounds.z <= wallMaxBounds.z) {
        return true; // Collision detected
    }
    return false; // No collision
}



int debugMode = 1;

const char* vertexShaderSource = R"(
    #version 330

    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec2 a_texture;
    layout(location = 2) in vec3 a_normal;

    uniform mat4 model;
    uniform mat4 projection;
    uniform mat4 view;

    out vec2 v_texture;

    void main()
    {
        gl_Position = projection * view * model * vec4(a_position, 1.0);
        v_texture = a_texture;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330

    in vec2 v_texture;

    out vec4 out_color;

    uniform sampler2D s_texture;

    void main()
    {
        out_color = texture(s_texture, v_texture);
    }
)";

void checkGLError() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error: " << error << std::endl;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        forward = true;
    }
    else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        forward = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        backward = true;
    }
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        backward = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        left = true;
    }
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        left = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        right = true;
    }
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        right = false;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    cam.ProcessMouseMovement(xoffset, 0.0f, true);
}

void doMovement() {
    if (left) {
        cam.ProcessKeyboard("LEFT", 0.015);
    }
    if (right) {
        cam.ProcessKeyboard("RIGHT", 0.015);
    }
    if (forward) {
        cam.ProcessKeyboard("FORWARD", 0.0015);
    }
    if (backward) {
        cam.ProcessKeyboard("BACKWARD", 0.015);
    }
}

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


int main() {
    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    glewExperimental = GL_TRUE;


    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "My OpenGL Window", nullptr, nullptr);

    if (!window) {
        std::cerr << "GLFW window creation failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }


    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetWindowSizeCallback(window, windowResizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check compilation status
    GLint vertexCompiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompiled);
    if (vertexCompiled != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertexShader, 1024, &logLength, message);
        // Write the error to a log or handle it accordingly
    }
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check compilation status
    GLint fragmentCompiled;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);
    if (fragmentCompiled != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragmentShader, 1024, &logLength, message);
        // Write the error to a log or handle it accordingly
    }

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    // Attach shaders to the program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Link the program
    glLinkProgram(shaderProgram);


    // Load 3D meshes
    std::pair<std::vector<uint32_t>, std::vector<float>> mazeModel = ObjLoader::loadModel("models/mazeY.obj", true);
    std::pair<std::vector<uint32_t>, std::vector<float>> agentModel = ObjLoader::loadModel("models/agentY.obj", true);
    std::pair<std::vector<uint32_t>, std::vector<float>> groundModel = ObjLoader::loadModel("models/groundY.obj", true);

    ObjWGroupsLoader objLoaderWGroups = ObjWGroupsLoader();
    std::vector<Mesh> MazeColliders = objLoaderWGroups.loadObj("models/mazeY_collider_NoTextures.obj");

    // Check if loading the OBJ file was successful
    if (MazeColliders.empty()) {
        std::cerr << "Error: Failed to load OBJ file." << std::endl;
        return -1; // or handle the error accordingly
    }

    // Initialize vectors to store vertices and indices
    std::vector<float> mazeCollidersBuffer;
    std::vector<uint32_t> mazeCollidersIndices;

    // Iterate over each mesh in MazeColliders
    for (const auto& mesh : MazeColliders) {
        // Iterate over each line of data in the mesh
        for (const auto& line : mesh.data) {
            // Check if the line represents a vertex
            if (line.substr(0, 2) == "v ") {
                // Extract vertex coordinates from the line
                float x, y, z;
                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                // Add vertex coordinates to the buffer
                mazeCollidersBuffer.push_back(x);
                mazeCollidersBuffer.push_back(y);
                mazeCollidersBuffer.push_back(z);
                //std::cout << "Line: " << line << std::endl;
                //std::cout << "Vertex: " << x << ", " << y << ", " << z << std::endl;

            }
            // Check if the line represents a face
            else if (line.substr(0, 2) == "f ") {
                // Extract face indices from the line
                uint32_t idx1, idx2, idx3;
                sscanf(line.c_str(), "f %u %u %u", &idx1, &idx2, &idx3);
                // OBJ file indices start from 1, so decrement them by 1 to get zero-based indices
                idx1--; idx2--; idx3--;
                // Add face indices to the indices vector
                mazeCollidersIndices.push_back(idx1);
                mazeCollidersIndices.push_back(idx2);
                mazeCollidersIndices.push_back(idx3);
                //std::cout << "Line: " << line << std::endl;
                //std::cout << "Indices: " << idx1 << ", " << idx2 << ", " << idx3 << std::endl;
            }
        }
    }



    // Access the loaded data
    std::vector<uint32_t> mazeIndices = mazeModel.first;
    std::vector<float> mazeBuffer = mazeModel.second;
    std::vector<uint32_t> agentIndices = agentModel.first;
    std::vector<float> agentBuffer = agentModel.second;
    std::vector<uint32_t> groundIndices = groundModel.first;
    std::vector<float> groundBuffer = groundModel.second;


    // Generate Vertex Array Objects (VAOs)
    glGenVertexArrays(4, VAO);
    //checkGLError();

    // Generate Vertex Buffer Objects (VBOs)
    glGenBuffers(4, VBO);
    //checkGLError();


    // Maze VAO
    glBindVertexArray(VAO[0]);
    // Bind the maze Vertex Buffer Object (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    // Copy mazeBuffer data to the GPU
    glBufferData(GL_ARRAY_BUFFER, mazeBuffer.size() * sizeof(float), &mazeBuffer[0], GL_STATIC_DRAW);
    // Set up vertex attribute pointers
    // maze vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // maze textures
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // maze normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));


    // Agent VAO
    glBindVertexArray(VAO[1]);
    // Bind the agent Vertex Buffer Object (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    // Copy agentBuffer data to the GPU
    glBufferData(GL_ARRAY_BUFFER, agentBuffer.size() * sizeof(float), &agentBuffer[0], GL_STATIC_DRAW);
    // Set up vertex attribute pointers
    // agent vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const GLvoid*>(0));
    // agent textures
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const GLvoid*>(12));
    // agent normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const GLvoid*>(20));

    // Ground VAO
    glBindVertexArray(VAO[2]);
    // Bind the ground Vertex Buffer Object (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    // Copy groundBuffer data to the GPU
    glBufferData(GL_ARRAY_BUFFER, groundBuffer.size() * sizeof(float), &groundBuffer[0], GL_STATIC_DRAW);
    // Set up vertex attribute pointers
    // ground vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // ground textures
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // ground normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    // Create and bind VAO for maze colliders
    GLuint mazeCollidersVAO, mazeCollidersVBO, mazeCollidersEBO;
    glGenVertexArrays(1, &mazeCollidersVAO);
    glGenBuffers(1, &mazeCollidersVBO);
    glGenBuffers(1, &mazeCollidersEBO);

    glBindVertexArray(mazeCollidersVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mazeCollidersVBO);
    glBufferData(GL_ARRAY_BUFFER, mazeCollidersBuffer.size() * sizeof(float), mazeCollidersBuffer.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mazeCollidersEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mazeCollidersIndices.size() * sizeof(uint32_t), mazeCollidersIndices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    //glBindVertexArray(0);


    glGenTextures(3, textures);
    TextureLoader::loadTexture("textures/maze.jpg", textures[0]);
    TextureLoader::loadTexture("textures/agent.jpg", textures[1]);
    TextureLoader::loadTexture("textures/ground.jpg", textures[2]);


    // Use the program
    glUseProgram(shaderProgram);
    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));



    glm::vec3 agentMinBounds(FLT_MAX);
    glm::vec3 agentMaxBounds(-FLT_MAX);

    for (size_t i = 0; i < agentBuffer.size(); i += 8) {
        glm::vec3 vertex(agentBuffer[i], agentBuffer[i + 1], agentBuffer[i + 2]);

        // Update minimum bounds
        agentMinBounds.x = std::min(agentMinBounds.x, vertex.x);
        agentMinBounds.y = std::min(agentMinBounds.y, vertex.y);
        agentMinBounds.z = std::min(agentMinBounds.z, vertex.z);

        // Update maximum bounds
        agentMaxBounds.x = std::max(agentMaxBounds.x, vertex.x);
        agentMaxBounds.y = std::max(agentMaxBounds.y, vertex.y);
        agentMaxBounds.z = std::max(agentMaxBounds.z, vertex.z);
    }




    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        doMovement();

        // Update the position of the agent
        glm::vec3 newAgentPos = agentPos;
        if (left) {
            newAgentPos.x -= agentSpeed;
        }
        if (right) {
            newAgentPos.x += agentSpeed;
        }
        if (forward) {
            newAgentPos.z -= agentSpeed;
        }
        if (backward) {
            newAgentPos.z += agentSpeed;
        }

        bool collisionDetected = false;

        // Iterate over each wall (sub-object) in MazeColliders
        for (const Mesh& wall : MazeColliders) {
            // Calculate bounding box of current wall
            glm::vec3 wallMinBounds, wallMaxBounds;
            std::tie(wallMinBounds, wallMaxBounds) = calculateBoundingBox(wall);

            // Check for collision between agent and current wall
            if (checkCollision(agentMinBounds, agentMaxBounds, wallMinBounds, wallMaxBounds)) {
                collisionDetected = true;
                std::cout << "Collision detected with wall: " << wall.name << std::endl;
            }
        }


        // Update the agent's position if no collision detected
        if (!collisionDetected) {
            agentPos = newAgentPos;
        }


        glm::mat4 view = cam.GetViewMatrix(agentPos);
        //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Draw the scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the maze
        glBindVertexArray(VAO[0]);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glm::mat4 cubePos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -10.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubePos));
        glDrawArrays(GL_TRIANGLES, 0, mazeIndices.size());
        glPopMatrix();

        // Draw the agent
        glBindVertexArray(VAO[1]);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        // Assuming modelLoc is the uniform location for the model matrix
        // btAgentPos is a vector containing the position of the agent
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(agentPos.x, agentPos.y, agentPos.z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glDrawArrays(GL_TRIANGLES, 0, agentIndices.size());
        glPopMatrix();

        // Draw the ground
        glBindVertexArray(VAO[2]);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glm::mat4 groundPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -10.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(groundPos));
        glDrawArrays(GL_TRIANGLES, 0, groundIndices.size());
        glPopMatrix();

        // Draw the colliders
        // Bind maze colliders VAO
        glBindVertexArray(mazeCollidersVAO);
        glm::mat4 collidersPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -10.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(collidersPos));
        glDrawArrays(GL_TRIANGLES, 0, mazeCollidersIndices.size());



        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(4, VAO);
    glDeleteBuffers(4, VBO);

    glfwTerminate();
    return 0;
}