//
// Created by AICDG on 2017/8/9.
//

#ifndef BULLETOPENGL_BULLETOPENGLAPPLICATION_H
#define BULLETOPENGL_BULLETOPENGLAPPLICATION_H

#include <GL/glew.h>
#define GLEW_STATIC
#include <GLFW/glfw3.h>

# include <GL/gl.h>
# include <GL/glu.h>

#include <Bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>

// include our custom Motion State object
#include "OpenGLMotionState.h"

#include "GameObject.h"
#include "Camera.h"
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
// Our custom debug renderer
#include "GLDebugDrawer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// a convenient typedef to reference an STL vector of GameObjects
typedef std::vector<GameObject*> GameObjects;

class BulletOpenGLApplication {
public:
	BulletOpenGLApplication();
	~BulletOpenGLApplication();
	void Initialize();
	// FreeGLUT callbacks //
	virtual void Keyboard(GLFWwindow* window, unsigned char key, int x, int y);
	virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	virtual void SpecialUp(GLFWwindow* window, int key, int x, int y);
	virtual void Reshape(GLFWwindow* window, int w, int h);
	virtual void Idle(GLFWwindow* window);
	virtual void Mouse(GLFWwindow* window, double xpos, double ypos);
	virtual void PassiveMotion(GLFWwindow* window, int x, int y);
	virtual void Motion(GLFWwindow* window, int x, int y);
	virtual void Display();

	// rendering. Can be overrideen by derived classes
	virtual void RenderScene();

	// scene updating. Can be overridden by derived classes
	virtual void UpdateScene(float dt);

	// physics functions. Can be overriden by derived classes (like BasicDemo)
	virtual void InitializePhysics() {};
	virtual void ShutdownPhysics() {};

	// camera functions
	void UpdateCamera();
	void RotateCamera(float& angle, float value);
	void ZoomCamera(float distance);

	// drawing functions
	void DrawBox(const btVector3& halfSize);
	void DrawShape(btScalar* transform, const btCollisionShape* pShape, const btVector3& color);

	// object functions
	GameObject* CreateGameObject(const std::string& objFilePath,
		const char* texturePath,
		glm::mat4 pos,
		btCollisionShape* pShape,
		const float& mass,
		const btVector3& color = btVector3(1.0f, 1.0f, 1.0f),
		const btVector3& initialPosition = btVector3(0.0f, 0.0f, 0.0f),
		const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1)
	);

	GameObject* CreateGameObject(
		glm::mat4 pos,
		btCollisionShape* pShape,
		const float& mass,
		const btVector3& color = btVector3(1.0f, 1.0f, 1.0f),
		const btVector3& initialPosition = btVector3(0.0f, 0.0f, 0.0f),
		const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1)
	);

	GLint projLoc;
	GLint modelLoc;
	GLint viewLoc;
	glm::mat4 projection;
	GLuint shaderProgram;
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
	bool firstMouse = true;

	bool left = false, right = false, forward = false, backward = false;
	float agentSpeed = 0.015f;
	GLfloat lastX = 1400 / 2.0f;
	GLfloat lastY = 800 / 2.0f;

protected:
	// camera control
	btVector3 m_cameraPosition; // the camera's current position
	btVector3 m_cameraTarget;	 // the camera's lookAt target
	float m_nearPlane; // minimum distance the camera will render
	float m_farPlane; // farthest distance the camera will render
	btVector3 m_upVector; // keeps the camera rotated correctly
	float m_cameraDistance; // distance from the camera to its target
	float m_cameraPitch; // pitch of the camera 
	float m_cameraYaw; // yaw of the camera
	Camera cam;

	int m_screenWidth;
	int m_screenHeight;

	// core Bullet components
	btBroadphaseInterface* m_pBroadphase;
	btCollisionConfiguration* m_pCollisionConfiguration;
	btCollisionDispatcher* m_pDispatcher;
	btConstraintSolver* m_pSolver;
	btDynamicsWorld* m_pWorld;

	// a simple clock for counting time
	btClock m_clock;

	// an array of our game objects
	GameObjects m_objects;

	// debug renderer
	DebugDrawer* m_pDebugDrawer;
};


#endif //BULLETOPENGL_BULLETOPENGLAPPLICATION_H