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
#include <Bullet/btBulletCollisionCommon.h>
#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/Serialize/BulletFileLoader/btBulletFile.h>


#include "ObjLoader.h"
#include "TextureLoader.h"
#include "Camera.h"
#include "GLDebugDrawer.h"
#include "OpenGLMotionState.h"


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

GLuint VAO[3];
GLuint VBO[3];
GLuint textures[3];

DebugDrawer* myDrawer;

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

// Bullet physics variables
btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

void initBullet() {
    // Create the broadphase
    broadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // The actual physics solver
    solver = new btSequentialImpulseConstraintSolver;

    // The world
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

    // Set gravity
    dynamicsWorld->setGravity(btVector3(0, -10, 0));


    //  set the debug drawer
    myDrawer = new DebugDrawer();
    dynamicsWorld->setDebugDrawer(myDrawer);

    myDrawer->setDebugMode(debugMode);

    // Optionally enable debug drawing - replace with your own key/button handling
    dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    // Set the debug drawer for the Bullet world
    dynamicsWorld->setDebugDrawer(myDrawer);
}

btRigidBody* createRigidBody(btCollisionShape* shape, btScalar mass, const btVector3& pos, bool isKinematic = false) {
    // Calculate inertia
    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(mass, inertia);

    btTransform initialTransform;
    initialTransform.setIdentity();
    initialTransform.setOrigin(pos);

    // Create motion state
    btDefaultMotionState* motionState = new OpenGLMotionState(initialTransform);

    // Create rigid body
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    body->clearForces();
    body->setLinearVelocity(btVector3(0, 0, 0));
    body->setWorldTransform(initialTransform);

    // objects of infinite mass can't
    // move or rotate
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, inertia);
    }


    // Set additional properties for the agent (kinematic)
    if (isKinematic) {
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        body->setActivationState(ACTIVE_TAG);
        body->setActivationState(DISABLE_DEACTIVATION);
    }

    // Add rigid body to the dynamics world
    dynamicsWorld->addRigidBody(body);

    return body;
}



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

    // Initialize Bullet
    initBullet();

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


    // Access the loaded data
    std::vector<uint32_t> mazeIndices = mazeModel.first;
    std::vector<float> mazeBuffer = mazeModel.second;
    std::vector<uint32_t> agentIndices = agentModel.first;
    std::vector<float> agentBuffer = agentModel.second;
    std::vector<uint32_t> groundIndices = groundModel.first;
    std::vector<float> groundBuffer = groundModel.second;


    // Generate Vertex Array Objects (VAOs)
    glGenVertexArrays(3, VAO);
    //checkGLError();

    // Generate Vertex Buffer Objects (VBOs)
    glGenBuffers(3, VBO);
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


    // Load the maze visual shape
    btTriangleMesh* mazeMesh = new btTriangleMesh();

    // Load the mesh from file
    btVector3 scaling(1, 1, 1);
    std::pair<std::vector<uint32_t>, std::vector<float>> mazeColliderModel = ObjLoader::loadModel("models/mazeY_collider.obj", true);

    // the vertices and indices are in the returned pair
    const std::vector<float>& vertices = mazeColliderModel.second;
    const std::vector<uint32_t>& indices = mazeColliderModel.first;

    for (size_t i = 0; i < indices.size(); i += 3) {
        // Indices point to the vertices in the 'vertices' array
        size_t index1 = indices[i];
        size_t index2 = indices[i + 1];
        size_t index3 = indices[i + 2];

        // Extract vertices from the 'vertices' array
        if (index1 * 8 + 2 < vertices.size()) {
            btVector3 vertex1(vertices[index1 * 8], vertices[index1 * 8 + 1], vertices[index1 * 8 + 2]);
            btVector3 vertex2(vertices[index2 * 8], vertices[index2 * 8 + 1], vertices[index2 * 8 + 2]);
            btVector3 vertex3(vertices[index3 * 8], vertices[index3 * 8 + 1], vertices[index3 * 8 + 2]);
            // Add the triangle to the mesh
            mazeMesh->addTriangle(vertex1, vertex2, vertex3, true);
        }
        else {
            // Handle the case where the index is out of bounds
            std::cerr << "Error: Invalid index1 in vertices vector." << std::endl;
        }
    }


    btCollisionShape* mazeShape = new btBvhTriangleMeshShape(mazeMesh, true, true);


    // Create Bullet collision shapes and rigid bodies for the other objects
    btCollisionShape* agentShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

    btRigidBody* mazeRigidBody = createRigidBody(mazeShape, 0, btVector3(0.0f, -4.0f, -10.0f));

    btVector3 agentInitialPos(agentPos.x, agentPos.y, agentPos.z);
    btRigidBody* agentRigidBody = createRigidBody(agentShape, 0.2, agentInitialPos, true);  // Set mass different than 0 for dynamic body

    btRigidBody* groundRigidBody = createRigidBody(groundShape, 0, btVector3(0.0f, -4.0f, -10.0f));


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



    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        doMovement();
        dynamicsWorld->stepSimulation(1 / 60.f, 10);
        dynamicsWorld->performDiscreteCollisionDetection();

        btVector3 btAgentPos(agentPos.x, agentPos.y, agentPos.z);
        if (left) {
            agentPos.x -= agentSpeed;
            btAgentPos.setX(btAgentPos.getX() - agentSpeed);
            btAgentPos.normalize();
            agentRigidBody->setLinearVelocity(btAgentPos);
        }
        if (right) {
            agentPos.x += agentSpeed;
            btAgentPos.setX(btAgentPos.getX() + agentSpeed);
            btAgentPos.normalize();
            agentRigidBody->setLinearVelocity(btAgentPos);
        }
        if (forward) {
            agentPos.z -= agentSpeed;
            btAgentPos.setZ(btAgentPos.getZ() - agentSpeed);
            btAgentPos.normalize();
            agentRigidBody->setLinearVelocity(btAgentPos);
        }
        if (backward) {
            agentPos.z += agentSpeed;
            btAgentPos.setZ(btAgentPos.getZ() + agentSpeed);
            btAgentPos.normalize();
            agentRigidBody->setLinearVelocity(btAgentPos);
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


        dynamicsWorld->stepSimulation(1 / 60.f, 10);


        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(3, VBO);

    // Clean up Bullet
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;

    // Clean up GLDebugDrawer
    delete myDrawer;

    glfwTerminate();
    return 0;
}