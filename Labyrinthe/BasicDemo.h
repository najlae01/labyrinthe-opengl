
#ifndef BULLETOPENGL_BASICDEMO_H
#define BULLETOPENGL_BASICDEMO_H

#include "BulletOpenGLApplication.h"
#include <Bullet/btBulletDynamicsCommon.h>

class BasicDemo : public BulletOpenGLApplication {
public:
	virtual void InitializePhysics() override;
	virtual void ShutdownPhysics() override;

	void CreateObjects();
};


#endif //BULLETOPENGL_BASICDEMO_H