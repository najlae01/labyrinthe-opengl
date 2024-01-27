//
// Created by AICDG on 2017/8/9.
//

#include "BasicDemo.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void BasicDemo::InitializePhysics() {
	// create the collision configuration
	m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
	// create the dispatcher
	m_pDispatcher = new btCollisionDispatcher(m_pCollisionConfiguration);
	// create the broadphase
	m_pBroadphase = new btDbvtBroadphase();
	// create the constraint solver
	m_pSolver = new btSequentialImpulseConstraintSolver();
	// create the world
	m_pWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pBroadphase, m_pSolver, m_pCollisionConfiguration);

	// create our scene's physics objects
	CreateObjects();
}

void BasicDemo::ShutdownPhysics() {
	delete m_pWorld;
	delete m_pSolver;
	delete m_pBroadphase;
	delete m_pDispatcher;
	delete m_pCollisionConfiguration;
}

void BasicDemo::CreateObjects() {
	// create a maze 
	glm::mat4 mazePos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -10.0f));
	CreateGameObject("models/mazeY.obj", "textures/maze.jpg", mazePos, new btBoxShape(btVector3(1, 50, 50)), 0, btVector3(0.2f, 0.6f, 0.6f), btVector3(0.0f, -4.0f, -10.0f));
	
	// create a ground plane
	glm::mat4 groundPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -10.0f));
	CreateGameObject("models/groundY.obj", "textures/ground.jpg", groundPos, new btBoxShape(btVector3(1, 50, 50)), 0, btVector3(0.2f, 0.6f, 0.6f), btVector3(0.0f, -4.0f, -10.0f));

	// create our original red box
	glm::mat4 agentPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -10.0f));
	CreateGameObject("models/agentY.obj", "textures/agent.jpg", agentPos, new btBoxShape(btVector3(1, 1, 1)), 1.0, btVector3(1.0f, 0.2f, 0.2f), btVector3(0.0f, -1.0f, -10.0f));

}