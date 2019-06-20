#pragma once

#include "Week9Application.h"

#include <vector>

typedef struct Vertex
{
	float position[3];
	float normal[3];
	float st[2];
} Vertex;

class Model
{
public:
	Model(char * filename);
	~Model(void);

protected:
	bool	loaddedSuccefully = false;
	bool	showWireframe = false;

	GLuint vertexBufferHandle;
	GLuint vertexArrayHandle;

	std::vector<unsigned short> newindices;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint elementArrayHandle;

public:
	bool	loadOK() { return loaddedSuccefully; }	// inline method
	void	setWireframe(bool status) { showWireframe = status; }
	bool	renderWireframe() { return showWireframe; }

	void render();

};
