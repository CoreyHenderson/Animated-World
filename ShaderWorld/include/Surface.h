#include "Week9Application.h"

#include "World.h"
#include "Model.h"

#pragma once

// The visualization element. 
class Surface
{
private:
	HWND handle;
	HDC hDC;
	void init();

	int		windowWidth;
	int		windowHeight;

	// Shadow Map data elements
	int		shadowMapWidth;
	int		shadowMapHeight;
	GLuint	shadowTextureBuffer;

	bool isOnEagleView;
	void setEagleView();

protected:
	// Link to the model; in this case the set
	// of models in the world that we're creating.
	World * world;

	glm::vec3 cameraPosition;
	glm::vec3 cameraForward;
	glm::vec3 cameraUp;

	float speed;
	float angleSpeed;

	mat3 rotationVectorAngle(float ux, float uy, float uz, float angle);
public:
	Surface(HINSTANCE hInst, HWND hWnd);
	~Surface(void);
	// resize surface as well as reset the graphics viewport.
	void resize(int width, int height);

	void initializeGL();
	
	void setupShadows();
	void setShadowMapBuffer();
	void restoreShadowMapBuffer(HWND hWnd);

	void paintGL();
	void resizeGL(int width, int height);
	
	// Move sideways.
	void strafe(float direction);
	void step(float direction);
	// Rotate
	void turnside(float direction);
	void turnup(float direction);
	// Move up or down
	void goUp(float direction);
	void goDown(float direction);
	// Change views
	void frontView();
	void backView();
	void leftView();
	void rightView();
	void topView();
	void enableEagleView();
	void resetOrientation();
};
