//
// Created by AICDG on 2017/8/9.
//

#include "GameObject.h"
#include "ObjLoader.h" 
#include "TextureLoader.h" 

#include <GL/glew.h>

# include <GL/gl.h>
# include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GameObject::GameObject(const std::string& objFilePath, const char* texturePath, glm::mat4 pos, btCollisionShape* pShape, float mass, const btVector3& color, const btVector3& initialPosition, const btQuaternion& initialRotation)
	: VAO(0), VBO(0), texture(0) {
	// store the shape for later usage
	m_pShape = pShape;

	// store the color
	m_color = color;

	m_pos = pos;

	LoadMesh(objFilePath, texturePath);

	// create the initial transform
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(initialPosition);
	transform.setRotation(initialRotation);

	// create the motion state from the
	// initial transform
	m_pMotionState = new OpenGLMotionState(transform);

	// calculate the local inertia
	btVector3 localInertia(0, 0, 0);

	// objects of infinite mass can't
	// move or rotate
	if (mass != 0.0f) {
		pShape->calculateLocalInertia(mass, localInertia);
	}

	// create the rigid body construction
	// info using the mass, motion state
	// and shape
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, m_pMotionState, pShape, localInertia);

	// create the rigid body
	m_pBody = new btRigidBody(cInfo);
}

GameObject::GameObject(glm::mat4 pos, btCollisionShape* pShape, float mass, const btVector3& color, const btVector3& initialPosition, const btQuaternion& initialRotation)
	: VAO(0), VBO(0), texture(0) {
	// store the shape for later usage
	m_pShape = pShape;

	// store the color
	m_color = color;

	m_pos = pos;

	// create the initial transform
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(initialPosition);
	transform.setRotation(initialRotation);

	// create the motion state from the
	// initial transform
	m_pMotionState = new OpenGLMotionState(transform);

	// calculate the local inertia
	btVector3 localInertia(0, 0, 0);

	// objects of infinite mass can't
	// move or rotate
	if (mass != 0.0f) {
		pShape->calculateLocalInertia(mass, localInertia);
	}

	// create the rigid body construction
	// info using the mass, motion state
	// and shape
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, m_pMotionState, pShape, localInertia);

	// create the rigid body
	m_pBody = new btRigidBody(cInfo);
}

void GameObject::LoadMesh(const std::string& objFilePath, const char* texturePath) {
	// Load OBJ model
	std::pair<std::vector<uint32_t>, std::vector<float>> model = ObjLoader::loadModel(objFilePath, true);
	indices = model.first;
	vertexBuffer = model.second;

	// Generate Vertex Array Object (VAO)
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Generate Vertex Buffer Object (VBO)
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), &vertexBuffer[0], GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// maze textures
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	// maze normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

	glGenTextures(1, &texture);
	TextureLoader::loadTexture(texturePath, texture);
}

void GameObject::drawObject(GLuint modelLoc) {
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_pos));
	glDrawArrays(GL_TRIANGLES, 0, indices.size());
}

GameObject::~GameObject() {
	delete m_pBody;
	delete m_pMotionState;
	delete m_pShape;
}