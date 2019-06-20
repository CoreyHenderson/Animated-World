#pragma once

#include "Week9Application.h"

class Shader
{
private:
	int program;
	int vertexShader;
	int geometryShader;
	int pixelShader;

	static int textureunit;

	// Data member to hold the texture identifiers (if any)
	// up to maximum of 3
#define MAXTEXTURES	3
	int	textures[MAXTEXTURES];

public:
	Shader(char * vertex, char * geomtery, char * pixel);
	~Shader(void);

	int		getProgram() { return program; }
	void	linkShaders();
	bool	loadShader(char * shaderfilename, int type, int & shader);
	void	activateShader();
	void	loadTexture(char * filename, int & texunit);
	void	assignTextureToSlot(char * filename, const int texslot);

	int		getTextureSlotID(const int textureSlot);
	void	setTextureSlotID(const int textureID, const int textureSlot);

	int		getTexture1() { return textures[0]; }
	int		getTexture2() { return textures[1]; }
	int		getTexture3() { return textures[2]; }

	int		setTexture1(int textureID) { return textures[0] = textureID; }
	int		setTexture2(int textureID) { return textures[1] = textureID; }
	int		setTexture3(int textureID) { return textures[2] = textureID; }
};

