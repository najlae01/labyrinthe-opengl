//
// Created by AICDG on 2017/8/9.
//

#ifndef BULLETOPENGL_GLFWCALLBACKS_H
#define BULLETOPENGL_GLFWCALLBACKS_H

#include "BulletOpenGLApplication.h"
#include <GL/glew.h>
#define GLEW_STATIC
#include <GLFW/glfw3.h>

// global pointer to our application object
static BulletOpenGLApplication* g_pApp;

// Various static functions that will be handed to GLFW to be called
// during various events (our callbacks). Each calls an equivalent function
// in our (global) application object.
static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    g_pApp->KeyCallback(window, key, scancode, action, mode);
}
static void KeyboardUpCallback(GLFWwindow* window, unsigned char key, int x, int y) {
    g_pApp->Keyboard(window, key, x, y);
}
static void SpecialCallback(GLFWwindow* window, int key, int x, int y) {
    g_pApp->SpecialUp(window, key, x, y);
}
static void SpecialUpCallback(GLFWwindow* window, int key, int x, int y) {
    g_pApp->SpecialUp(window, key, x, y);
}
static void ReshapeCallback(GLFWwindow* window, int width, int height) {
    g_pApp->Reshape(window, width, height);
}
static void IdleCallback(GLFWwindow* window) {
    g_pApp->Idle(window);
}
static void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    g_pApp->Mouse(window, xpos, ypos);
}
static void MotionCallback(GLFWwindow* window, int x, int y) {
    g_pApp->Motion(window, x, y);
}
static void DisplayCallback(void) {
    g_pApp->Display();
}

// our custom-built 'main' function, which accepts a reference to a
// BulletOpenGLApplication object.
int glfwmain(int width, int height, BulletOpenGLApplication* pApp) {
    // store the application object so we can
    // access it globally
    g_pApp = pApp;

    glewExperimental = GL_TRUE;

    // initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(width, height, "Labyrinthe", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // make the window's context current
    glfwMakeContextCurrent(window);

    // initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1;
    }

    // perform custom initialization of our application
    g_pApp->Initialize();

    // set GLFW callback functions
    glfwSetKeyCallback(window, KeyboardCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetWindowSizeCallback(window, ReshapeCallback);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    // perform one render before we launch the application
    g_pApp->Idle(window);


    // loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // poll for and process events
        //glfwPollEvents();
        
        // render here
        g_pApp->Idle(window);

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll for and process events
        glfwPollEvents();
    }

    // cleanup and exit
    glfwTerminate();
    return 0;
}

#endif //BULLETOPENGL_GLFWCALLBACKS_H
