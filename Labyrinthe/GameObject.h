//
// Created by AICDG on 2017/8/9.
//

#ifndef BULLETOPENGL_GAMEOBJECT_H
#define BULLETOPENGL_GAMEOBJECT_H

#include <Bullet/btBulletDynamicsCommon.h>
#include "OpenGLMotionState.h"
#include <vector>

#include <GL/glew.h>

# include <GL/gl.h>
# include <GL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



class GameObject {
public:
	GameObject(const std::string& objFilePath, const char* texturePath, glm::mat4 pos, btCollisionShape* pShape, float mass, const btVector3& color, const btVector3& initialPosition = btVector3(0, 0, 0), const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1));

	GameObject(glm::mat4 pos, btCollisionShape* pShape, float mass, const btVector3& color, const btVector3& initialPosition = btVector3(0, 0, 0), const btQuaternion& initialRotation = btQuaternion(0, 0, 1, 1));

	~GameObject();

	// accessors
	btCollisionShape* GetShape() { return m_pShape; }

	btRigidBody* GetRigidBody() { return m_pBody; }

	btMotionState* GetMotionState() { return m_pMotionState; }

	void GetTransform(btScalar* transform) {
		if (m_pMotionState) m_pMotionState->GetWorldTransform(transform);
	}

	glm::mat4 GetPosition() {
		return m_pos;
	}

	btVector3 GetColor() { return m_color; }

	void drawObject(GLint& modelLoc);

private:	

	// New private function to load and initialize the mesh
	void LoadMesh(const std::string& objFilePath, const char* texturePath);


protected:
	btCollisionShape* m_pShape;
	btRigidBody* m_pBody;
	OpenGLMotionState* m_pMotionState;
	btVector3      m_color;
	GLuint VAO;
	GLuint VBO;
	GLuint texture;
	std::vector<uint32_t> indices;
	std::vector<float> vertexBuffer;
	glm::mat4 m_pos;
};


#endif //BULLETOPENGL_GAMEOBJECT_H