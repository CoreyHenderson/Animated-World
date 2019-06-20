#include "Week9Application.h"

#include "stdafx.h"
#include "Shader.h"
#include "Model.h"
#include "World.h"

WorldObject::WorldObject(Model * model, Shader * shader	) : model(model), shader(shader)
{
	setPosition(0.0f, 0.0f, 0.0f);	// Origin 
	setForward(0.0f, 0.0f, 1.0f);	// Forwrd vector along -ve z-axis
	setScale(1.0f, 1.0f, 1.0f);		// Scale at 1x
	
	// Initialise 'up' vector to be along +ve y-axis
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
}

void WorldObject::setPosition(float x, float y, float z)
{
	modelPosition.x = x;
	modelPosition.y = y;
	modelPosition.z = z;
	return;
}

void WorldObject::setForward(float forwardx, float forwardy, float forwardz)
{
	forward.x = forwardx;
	forward.y = forwardy;
	forward.z = forwardz;
	return;
}

void WorldObject::setScale(float sx, float sy, float sz )
{
	scale.x = sx;
	scale.y = sy;
	scale.z = sz;
	return;
}

void WorldObject::setUp(float ux, float uy, float uz)
{
	up.x = ux;
	up.y = uy;
	up.z = uz;
	return;
}

vec3 WorldObject::getForward()
{
	return forward;
}

vec3 WorldObject::getUp()
{
	return up;
}

vec3 WorldObject::getPosition()
{
	return modelPosition;
}

mat4 WorldObject::getWorldTransform()
{
	vec3 dirside = glm::cross(up, forward);

	// in this format, goes in as the transpose.
	mat4 Rotation = mat4(	dirside[0], up.x, forward.x, 0.0,
							dirside[1], up.y, forward.y, 0.0,
							dirside[2], up.z, forward.z, 0.0,
							0.0, 0.0, 0.0, 1.0 );
	mat4 Scale = mat4(	scale.x, 0, 0, 0,
						0, scale.y, 0, 0,
						0, 0, scale.z, 0,
						0, 0, 0, 1 );

	mat4 M = glm::translate(mat4(1.0f), modelPosition) * Rotation * Scale;
	return M;
}



World::World(void)
{
	clock = 0.0;

	// Create an array of pointers (fixed size) to hold our models
	const int maxModels = 30;
	models = new WorldObject *[maxModels];
	numModels = 0;

	// Load in each of the models we will be using in this scene.
	Model * skybox = new Model((char *) "skydome.obj");
	Model * ground = new Model((char *) "plane.obj");
	Model * boat = new Model((char *) "boat.obj");
	Model * oar = new Model((char *) "oar.obj");
	Model * eagle = new Model((char *) "eagle.obj");
	Model * tree1 = new Model((char *) "tree1.obj");
	Model * tree2 = new Model((char *) "tree2.obj");
	Model * rock1 = new Model((char *) "rock1.obj");
	Model * rock2 = new Model((char *) "rock2.obj");
	Model * rock3 = new Model((char *) "rock3.obj");

	// Create all the shader combinations for each object in the scene
	Shader * skyboxShader = new Shader((char *) "model.vert", (char *) "empty.geom", (char *) "sky.frag");
	Shader * groundShader = new Shader((char *) "ground.vert", (char *) "ground.geom", (char *) "ground.frag");
	Shader * boatShader  = new Shader((char *) "boat.vert", (char *) "empty.geom", (char *) "boat.frag");
	Shader * oar1Shader  = new Shader((char *) "oar1.vert", (char *) "empty.geom", (char *) "oar.frag");
	Shader * oar2Shader  = new Shader((char *) "oar2.vert", (char *) "empty.geom", (char *) "oar.frag");
	Shader * tree1Shader = new Shader((char *) "model.vert", (char *) "empty.geom", (char *) "tree1.frag");
	Shader * tree2Shader = new Shader((char *) "model.vert", (char *) "empty.geom", (char *) "tree2.frag");
	Shader * rock1Shader = new Shader((char *) "model.vert", (char *) "empty.geom", (char *) "rock1.frag");
	Shader * rock2Shader = new Shader((char *) "model.vert", (char *) "empty.geom", (char *) "rock2.frag");
	Shader * eagleShader = new Shader((char *) "eagle.vert", (char *) "empty.geom", (char *) "eagle.frag");

	// Assign textures to the shaders
	skyboxShader->assignTextureToSlot("SkyFix.png", 0);
	groundShader->assignTextureToSlot("heightmap.png", 0);
	boatShader->assignTextureToSlot("boat.png", 0);
	boatShader->assignTextureToSlot("boatnormal.png", 1);
	oar1Shader->assignTextureToSlot("oar.jpg", 0);
	oar2Shader->assignTextureToSlot("oar.jpg", 0);
	eagleShader->assignTextureToSlot("eagle.png", 0);

	/***************************** SkyBox *****************************/
	models[numModels] = new WorldObject(skybox, skyboxShader);
	models[numModels]->setPosition(0.0f, 0.0f, 0.0f);
	models[numModels]->setForward(0.0f, 0.0f, -1.0f);
	models[numModels]->setScale(1.0f, 1.0f, 1.0f);
	numModels++;
	/*********************** Ocean & Mountain *************************/
	models[numModels] = new WorldObject(ground, groundShader);
	models[numModels]->setPosition(0.0f, 2.0f, 0.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(25.0f, 25.0f, 25.0f);
	numModels++;
	/***************************** Boat *******************************/
	// Boat
	models[numModels] = new WorldObject(boat, boatShader);
	models[numModels]->setPosition(+3.8f, 3.1f, -21.0f);
	models[numModels]->setForward(+1.0f, 0.0f, 0.0f);
	models[numModels]->setScale(0.2f, 0.2f, 0.2f);
	numModels++;
	// Boat Paddles
	models[numModels] = new WorldObject(oar, oar1Shader);
	models[numModels]->setPosition(+1.5f, 3.07f, -22.5f);
	models[numModels]->setForward(-1.0f, 0.0f, 1.0f);
	models[numModels]->setScale(0.03f, 0.03f, 0.03f);
	numModels++;
	models[numModels] = new WorldObject(oar, oar2Shader);
	models[numModels]->setPosition(+4.00, 3.38f, -21.30f);
	models[numModels]->setForward(1.0f, 0.0f, -1.0f);
	models[numModels]->setScale(0.03f, 0.03f, 0.03f);
	numModels++;
	/***************************** Eagle *****************************/
	models[numModels] = new WorldObject(eagle, eagleShader);
	models[numModels]->setPosition(0.00, 15.0f, 10.00f);
	models[numModels]->setForward(-1.0f, 0.0f, 0.0f);
	models[numModels]->setScale(1.5f, 1.5f, 1.5f);
	numModels++;
	/***************************** Trees *****************************/
	// Island Trees - Count: 9
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(5.0f, 5.0f, 20.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.08f, 0.08f, 0.08f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(8.0f, 6.12f, 19.0f);
	models[numModels]->setForward(0.0, 0.0, -1.0f);
	models[numModels]->setScale(0.11f, 0.11f, 0.11f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(8.25f, 3.40f, -6.2f);
	models[numModels]->setForward(+1.0, 0.0, 0.0f);
	models[numModels]->setScale(0.09f, 0.09f, 0.09f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(20.0f, 4.2f, 5.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.06f, 0.07f, 0.06f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(21.0f, 3.65f, 7.0f);
	models[numModels]->setForward(-1.0, 0.0, +1.0f);
	models[numModels]->setScale(0.04f, 0.04f, 0.04f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(19.0f, 4.5f, 5.0f);
	models[numModels]->setForward(-1.0, 0.0, 0.0f);
	models[numModels]->setScale(0.045f, 0.055f, 0.045f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(20.0f, 3.75f, 2.0f);
	models[numModels]->setForward(+1.0, 0.0, 0.0f);
	models[numModels]->setScale(0.035f, 0.035f, 0.035f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(18.0f, 3.45f, 9.5f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.04f, 0.04f, 0.04f);
	numModels++;
	models[numModels] = new WorldObject(tree1, tree1Shader);
	models[numModels]->setPosition(-9.0f, 4.4f, 9.0f);
	models[numModels]->setForward(-1.0, 0.0, 0.0f);
	models[numModels]->setScale(0.08f, 0.08f, 0.08f);
	numModels++;
	// Pine Trees - Count: 7
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(-5.0f, 3.95f, 24.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.08f, 0.08f, 0.08f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(-8.0f, 3.85f, 24.5f);
	models[numModels]->setForward(0.0, 0.0, -1.0f);
	models[numModels]->setScale(0.08f, 0.08f, 0.08f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(-4.0f, 4.05f, 23.0f);
	models[numModels]->setForward(1.0, 0.0, 0.0f);
	models[numModels]->setScale(0.1f, 0.1f, 0.1f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(14.0f, 3.55f, 21.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.09f, 0.09f, 0.09f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(13.0f, 3.37f, 23.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.06f, 0.06f, 0.06f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(12.0f, 3.55f, -12.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.07f, 0.07f, 0.07f);
	numModels++;
	models[numModels] = new WorldObject(tree2, tree2Shader);
	models[numModels]->setPosition(14.0f, 3.32f, -15.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.05f, 0.05f, 0.05f);
	numModels++;
	/***************************** Rocks *****************************/
	// South Mountain Side - Count: 3
	models[numModels] = new WorldObject(rock1, rock1Shader); // Big Rock
	models[numModels]->setPosition(-3.5f, 6.0f, -8.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.3f, 0.3f, 0.3f);
	numModels++;
	models[numModels] = new WorldObject(rock2, rock2Shader); // Small Rock
	models[numModels]->setPosition(-5.0f, 6.4f, -8.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.3f, 0.3f, 0.3f);
	numModels++;
	models[numModels] = new WorldObject(rock3, rock2Shader); // Flat Rock
	models[numModels]->setPosition(-5.0f, 6.0f, -9.0f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.3f, 0.3f, 0.3f);
	numModels++;
	// South East Ocean Side - Count: 4
	models[numModels] = new WorldObject(rock2, rock2Shader);
	models[numModels]->setPosition(-11.0f, 2.8f, -15.9f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.2f, 0.2f, 0.2f);
	numModels++;
	models[numModels] = new WorldObject(rock2, rock2Shader);
	models[numModels]->setPosition(-10.3f, 2.9f, -15.8f);
	models[numModels]->setForward(0.0, 0.0, -1.0f);
	models[numModels]->setScale(0.2f, 0.2f, 0.2f);
	numModels++;
	models[numModels] = new WorldObject(rock3, rock2Shader);
	models[numModels]->setPosition(-10.0f, 2.7f, -19.0f);
	models[numModels]->setForward(0.0, 0.0, 1.0f);
	models[numModels]->setScale(0.2f, 0.2f, 0.2f);
	numModels++;
	models[numModels] = new WorldObject(rock3, rock2Shader);
	models[numModels]->setPosition(-10.0f, 2.7f, -17.0f);
	models[numModels]->setForward(1.0, 0.0, +0.0f);
	models[numModels]->setScale(0.3f, 0.3f, 0.3f);
	numModels++;
	models[numModels] = new WorldObject(rock3, rock2Shader);
	models[numModels]->setPosition(-3.0f, 2.9f, -19.7f);
	models[numModels]->setForward(0.0, 0.0, +1.0f);
	models[numModels]->setScale(0.15f, 0.15f, 0.15f);
	numModels++;

	// This is an error checking - to force a break, if the numModels exceeds our maximum number of models
	assert(numModels <= maxModels);

	// Initialise the light position
	lx = 00.0;
	ly = 42.0;
	lz = 00.0;
}


World::~World(void)
{

}

void World::renderSpecific(WorldObject * object, mat4 view, mat4 proj, mat4 shadow)
{
	static float pauto = 0.0f;		// Define a global variable that is only available to this method.

	Model * model = object->getModel();
	mat4 world = object->getWorldTransform();
	Shader * shader = object->getShader();

	shader->activateShader();

	glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), (char *) "WorldMatrix"), 1, GL_FALSE, &world[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), (char *) "ViewMatrix"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), (char *) "ProjectionMatrix"), 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->getProgram(), (char *) "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

	glUniform1i(glGetUniformLocation(shader->getProgram(), (char *) "ShadowMap"), 0);
	glUniform4f(glGetUniformLocation(shader->getProgram(), (char *) "LightPosition"), lx, ly, lz, 1.0f);

	vec2 windowSize = getWindowSize();
	glUniform2f(glGetUniformLocation(shader->getProgram(), (char *) "u_resolution"), windowSize.x, windowSize.y);
	
	// Pass in an incrementing counter to the shader
	glUniform1f(glGetUniformLocation(shader->getProgram(), (char *) "pauto"), pauto);
	pauto += 0.1f;
	
	glUniform1i(glGetUniformLocation(shader->getProgram(), "texture1"), shader->getTexture1());
	glUniform1i(glGetUniformLocation(shader->getProgram(), "texture2"), shader->getTexture2());
	glUniform1i(glGetUniformLocation(shader->getProgram(), "texture3"), shader->getTexture3());

	model->render();
}

void World::animateEagle(double clock)
{
	int eagleIndex = 5;

	// Calculate position
	float radius = 13;
	float xPos = radius * (float)sin(clock);
	float yPos = sin(clock / 1.0f) * 3.0f + 13;
	float zPos = radius * (float)cos(clock);

	models[eagleIndex]->setPosition(xPos, yPos, zPos);		// Set position

	// Calculate Up
	vec3 up = models[eagleIndex]->getUp();
	models[eagleIndex]->setUp(-0.5f, up.y, up.z);			// Set Up (Roll)

	// Calculate rotation
	vec3 dirforward = normalize(models[eagleIndex]->getForward());
	vec3 dirup = normalize(models[eagleIndex]->getUp());

	float angle = 0.01;

	float ct = cos(angle);
	float st = sin(angle);
	float oct = 1.0f - ct;
	float ost = 1.0f - st;
	mat3 R = mat3(ct + dirup.x * dirup.x * oct, dirup.x * dirup.y * oct - dirup.z * st, dirup.x * dirup.z * oct + dirup.y * st,
				  dirup.y * dirup.x * oct + dirup.z * st, ct + dirup.y * dirup.y * oct, dirup.y * dirup.z * oct - dirup.x * st,
				  dirup.z * dirup.x * oct - dirup.y * st, dirup.z * dirup.y * oct + dirup.x * st, ct + dirup.z * dirup.z * oct);

	vec3 newRotation = R * dirforward;
	models[eagleIndex]->setForward(newRotation.x, newRotation.y, newRotation.z);		// Set rotation
}

void World::render(mat4 view, mat4 proj, mat4 shadow)
{
	static double counter = 0.0;

	static const float RADIUS = 90.0f;						// Wall of the skybox
	static const float lightStartingPosition = 66.30f;		// Position of the sun on the skybox
	static const float lightRotationSpeed = 15.90f;			// Rotating speed of the texture on the skybox

	clock = counter;

	// Update the Light Position - circular path in X/Z plane which matches the sun's position on the lightbox texture
	lx = RADIUS * (float)-sin( (clock + lightStartingPosition) / lightRotationSpeed );
	lz = RADIUS * (float)cos(  (clock + lightStartingPosition) / lightRotationSpeed );

	// Sets the last object in the array (rock) as the light's position for testing if the light's position matches the sun on the skybox
	//models[29]->setPosition(lx, ly, lz);

	// Update Eagle position, rotation and roll vectors
	animateEagle(clock);

	for (int i = 0; i < numModels; i++)
	{
		renderSpecific(models[i], view, proj, shadow);
	}

	counter += 0.01;
}

vec3 World::getEaglePosition()
{
	return models[5]->getPosition();
}

vec3 World::getEagleRotation()
{
	return models[5]->getForward();
}

double World::getClock()
{
	return clock;
}
