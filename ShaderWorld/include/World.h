#pragma once

#include "Week9Application.h"

class Model;
class Shader;

class WorldObject
{
private:
	Model * model;
	Shader * shader;
	vec3	modelPosition;
	vec3	forward;
	vec3	up;
	vec3	scale;

public:
	WorldObject(Model * model, Shader * shader );
	~WorldObject();
	void setPosition(float x, float y, float z);
	void setForward(float forwardx, float forwardy, float forwardz);
	void setScale(float scalex, float scaley, float scalez);
	void setUp(float ux, float uy, float uz);
	vec3 getForward();
	vec3 getUp();
	vec3 getPosition();
	mat4 getWorldTransform();

	Model * getModel() { return model; }
	Shader * getShader() { return shader; }
};

class World
{
private:
	WorldObject * * models;
	int numModels;
	int sunModelNumber;		// Holds the number of the model used to represent the sun!

	// Light posistion data
	float lx;
	float ly;
	float lz;
	//vec3	lightPosn;
	
	vec2	windowSize;

	double clock;

	void animateEagle(double clock);

	void renderSpecific(WorldObject * object, mat4 view, mat4 proj, mat4 shadow);

public:
	World(void);
	~World(void);

	void setWindowSize(int width, int height) { windowSize.x = float(width); windowSize.y = float(height); return; };
	vec2 getWindowSize() { return windowSize; };

	void render (mat4 view, mat4 proj, mat4 shadow);

	vec3 getEaglePosition();
	vec3 getEagleRotation();
	double getClock();
};
