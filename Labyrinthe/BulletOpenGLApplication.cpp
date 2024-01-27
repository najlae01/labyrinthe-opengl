//
// Created by AICDG on 2017/8/9.
//

#include "BulletOpenGLApplication.h"
#include <iostream>
#include <GL/glew.h>
#define GLEW_STATIC
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Some constants for 3D math and the camera speed
#define RADIANS_PER_DEGREE 0.01745329f
#define CAMERA_STEP_SIZE 5.0f

BulletOpenGLApplication::BulletOpenGLApplication() :
	m_cameraPosition(10.0f, 5.0f, 0.0f),
	m_cameraTarget(0.0f, 0.0f, 0.0f),
	m_cameraDistance(15.0f),
	m_cameraPitch(20.0f),
	m_cameraYaw(0.0f),
	m_upVector(0.0f, 1.0f, 0.0f),
	m_nearPlane(1.0f),
	m_farPlane(1000.0f),
	m_pBroadphase(nullptr),
	m_pCollisionConfiguration(nullptr),
	m_pDispatcher(nullptr),
	m_pSolver(nullptr),
	m_pWorld(nullptr)
{

}

BulletOpenGLApplication::~BulletOpenGLApplication() {
	// shutdown the physics system
	ShutdownPhysics();
}

void BulletOpenGLApplication::Initialize() {
	// this function is called inside glutmain() after
	// creating the window, but before handing control
	// to glfw
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
	shaderProgram = glCreateProgram();
	// Attach shaders to the program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	// Link the program
	glLinkProgram(shaderProgram);

	// initialize the physics system
	InitializePhysics();

	// Use the program
	glUseProgram(shaderProgram);
	glClearColor(0.0f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	modelLoc = glGetUniformLocation(shaderProgram, "model");
	projLoc = glGetUniformLocation(shaderProgram, "projection");
	viewLoc = glGetUniformLocation(shaderProgram, "view");

	projection = glm::perspective(glm::radians(45.0f), (float)1400 / (float)800, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	// create the debug drawer
	m_pDebugDrawer = new DebugDrawer();
	// set the initial debug level to 0
	m_pDebugDrawer->setDebugMode(0);
	// add the debug drawer to the world
	m_pWorld->setDebugDrawer(m_pDebugDrawer);
}

void BulletOpenGLApplication::Keyboard(GLFWwindow* window, unsigned char key, int x, int y) {
	// This function is called by FreeGLUT whenever
	// generic keys are pressed down.
	switch (key) {
		// 'z' zooms in
	case 'z': ZoomCamera(+CAMERA_STEP_SIZE); break;
		// 'x' zoom out
	case 'x': ZoomCamera(-CAMERA_STEP_SIZE); break;
	case 'w':
		// toggle wireframe debug drawing
		m_pDebugDrawer->ToggleDebugFlag(btIDebugDraw::DBG_DrawWireframe);
		break;
	case 'b':
		// toggle AABB debug drawing
		m_pDebugDrawer->ToggleDebugFlag(btIDebugDraw::DBG_DrawAabb);
		break;
	}
}

void BulletOpenGLApplication::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		// toggle wireframe debug drawing
		std::cout << "toggle wireframe debug drawing" << std::endl;
		m_pDebugDrawer->ToggleDebugFlag(btIDebugDraw::DBG_DrawWireframe);
	}
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
void BulletOpenGLApplication::SpecialUp(GLFWwindow* window, int key, int x, int y) {}

void BulletOpenGLApplication::Reshape(GLFWwindow* window, int w, int h) {
	glViewport(0, 0, w, h);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//UpdateCamera();
}

void BulletOpenGLApplication::Idle(GLFWwindow* window) {
	// this function is called frequently, whenever FreeGlut
	// isn't busy processing its own events. It should be used
	// to perform any updating and rendering tasks


	// get the time since the last iteration
	float dt = m_clock.getTimeMilliseconds();
	// reset the clock to 0
	m_clock.reset();
	// update the scene (convert ms to s)
	UpdateScene(dt * 100000.0f);

	// update the camera
	UpdateCamera();

	// render the scene
	RenderScene();

	// swap the front and back buffers
	glfwSwapBuffers(window);
}

void BulletOpenGLApplication::Mouse(GLFWwindow* window, double xpos, double ypos) {
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
void BulletOpenGLApplication::PassiveMotion(GLFWwindow* window, int x, int y) {}
void BulletOpenGLApplication::Motion(GLFWwindow* window, int x, int y) {}
void BulletOpenGLApplication::Display() {}

void BulletOpenGLApplication::UpdateCamera() {
	// exit in erroneous situations
	if (m_screenWidth == 0 && m_screenHeight == 0)
		return;

	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = cam.GetViewMatrix(glm::vec3(0.0f, -3.0f, -10.0f));
	//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	// the view matrix is now set
}

void BulletOpenGLApplication::DrawBox(const btVector3& halfSize) {

	float halfWidth = halfSize.x();
	float halfHeight = halfSize.y();
	float halfDepth = halfSize.z();

	// create the vertex positions
	btVector3 vertices[8] = {
	btVector3(halfWidth,  halfHeight,  halfDepth),
	btVector3(-halfWidth,  halfHeight,  halfDepth),
	btVector3(halfWidth, -halfHeight,  halfDepth),
	btVector3(-halfWidth, -halfHeight,  halfDepth),
	btVector3(halfWidth,  halfHeight, -halfDepth),
	btVector3(-halfWidth,  halfHeight, -halfDepth),
	btVector3(halfWidth, -halfHeight, -halfDepth),
	btVector3(-halfWidth, -halfHeight, -halfDepth) };

	// create the indexes for each triangle, using the
	// vertices above. Make it static so we don't waste 
	// processing time recreating it over and over again
	static int indices[36] = {
		0,1,2,
		3,2,1,
		4,0,6,
		6,0,2,
		5,1,4,
		4,1,0,
		7,3,1,
		7,1,5,
		5,4,7,
		7,4,6,
		7,2,3,
		7,6,2 };

	// start processing vertices as triangles
	glBegin(GL_TRIANGLES);

	// increment the loop by 3 each time since we create a
	// triangle with 3 vertices at a time.

	for (int i = 0; i < 36; i += 3) {
		// get the three vertices for the triangle based
		// on the index values set above
		// use const references so we don't copy the object
		// (a good rule of thumb is to never allocate/deallocate
		// memory during *every* render/update call. This should 
		// only happen sporadically)
		const btVector3& vert1 = vertices[indices[i]];
		const btVector3& vert2 = vertices[indices[i + 1]];
		const btVector3& vert3 = vertices[indices[i + 2]];

		// create a normal that is perpendicular to the
		// face (use the cross product)
		btVector3 normal = (vert3 - vert1).cross(vert2 - vert1);
		normal.normalize();

		// set the normal for the subsequent vertices
		glNormal3f(normal.getX(), normal.getY(), normal.getZ());

		// create the vertices
		glVertex3f(vert1.x(), vert1.y(), vert1.z());
		glVertex3f(vert2.x(), vert2.y(), vert2.z());
		glVertex3f(vert3.x(), vert3.y(), vert3.z());
	}

	// stop processing vertices
	glEnd();
}

void BulletOpenGLApplication::RotateCamera(float& angle, float value) {
	// change the value (it is passed by reference, so we
	// can edit it here)
	angle -= value;
	// keep the value within bounds
	if (angle < 0) angle += 360;
	if (angle >= 360) angle -= 360;
	// update the camera since we changed the angular value
	UpdateCamera();
}

void BulletOpenGLApplication::ZoomCamera(float distance) {
	// change the distance value
	m_cameraDistance -= distance;
	// prevent it from zooming in too far
	if (m_cameraDistance < 0.1f) m_cameraDistance = 0.1f;
	// update the camera since we changed the zoom distance
	UpdateCamera();
}

void BulletOpenGLApplication::RenderScene() {
	
	// iterate through all of the objects in our world
	for (GameObjects::iterator i = m_objects.begin(); i != m_objects.end(); ++i) {
		// get the object from the iterator
		GameObject* pObj = *i;

		// draw the object
		//std::cout << "Drawing object n" << pObj << std::endl;
		pObj->drawObject(modelLoc);
	}

	// after rendering all game objects, perform debug rendering
	// Bullet will figure out what needs to be drawn then call to
	// our DebugDrawer class to do the rendering for us
	m_pWorld->debugDrawWorld();
}

void BulletOpenGLApplication::UpdateScene(float dt) {
	// check if the world object exists
	if (m_pWorld) {
		float targetFPS = 60.0f;
		float timeStep = 1.0f / targetFPS;

		// Choose maxSubSteps based on your trade-off between accuracy and performance
		int maxSubSteps = 1;

		// Choose fixedTimeStep based on your desired accuracy
		btScalar fixedTimeStep = 1.0f / targetFPS;
		// step the simulation through time. This is called
		// every update and the amount of elasped time was 
		// determined back in ::Idle() by our clock object.
		m_pWorld->stepSimulation(dt, maxSubSteps, fixedTimeStep);
	}
}

void BulletOpenGLApplication::DrawShape(btScalar* transform, const btCollisionShape* pShape, const btVector3& color) {
	// set the color
	glColor3f(color.x(), color.y(), color.z());

	// push the matrix stack
	glPushMatrix();
	glMultMatrixf(transform);

	// make a different draw call based on the object type
	switch (pShape->getShapeType()) {
		// an internal enum used by Bullet for boxes
	case BOX_SHAPE_PROXYTYPE:
	{
		// assume the shape is a box, and typecast it
		const btBoxShape* box = static_cast<const btBoxShape*>(pShape);
		// get the 'halfSize' of the box
		btVector3 halfSize = box->getHalfExtentsWithMargin();
		// draw the box
		DrawBox(halfSize);
		break;
	}
	default:
		// unsupported type
		break;
	}

	// pop the stack
	glPopMatrix();
}

GameObject* BulletOpenGLApplication::CreateGameObject(const std::string& objFilePath, const char* texturePath, glm::mat4 pos, btCollisionShape* pShape, const float& mass, const btVector3& color, const btVector3& initialPosition, const btQuaternion& initialRotation ) {
	// create a new game object
	GameObject* pObject = new GameObject(objFilePath, texturePath, pos, pShape, mass, color, initialPosition, initialRotation);

	// push it to the back of the list
	m_objects.push_back(pObject);

	// check if the world object is valid
	if (m_pWorld) {
		// add the object's rigid body to the world
		m_pWorld->addRigidBody(pObject->GetRigidBody());
	}
	return pObject;
}

GameObject* BulletOpenGLApplication::CreateGameObject(glm::mat4 pos, btCollisionShape* pShape, const float& mass, const btVector3& color, const btVector3& initialPosition, const btQuaternion& initialRotation) {
	// create a new game object
	GameObject* pObject = new GameObject(pos, pShape, mass, color, initialPosition, initialRotation);

	// push it to the back of the list
	m_objects.push_back(pObject);

	// check if the world object is valid
	if (m_pWorld) {
		// add the object's rigid body to the world
		m_pWorld->addRigidBody(pObject->GetRigidBody());
	}
	return pObject;
}