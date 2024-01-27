//
// Created by AICDG on 2017/8/9.
//

#include "BasicDemo.h"
#include "GLFWCallbacks.h"

int main(int argc, char** argv) {
    BasicDemo demo;
    return glfwmain(1400, 800, &demo);
}