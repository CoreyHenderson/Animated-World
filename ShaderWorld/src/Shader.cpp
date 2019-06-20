
#include "Week9Application.h"

#include "stdafx.h"

#include "Shader.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <CommCtrl.h>

#include "LoadPNG.h"

using namespace std;

int Shader::textureunit = 0;

Shader::Shader(char * vertex, char * geometry, char * pixel)
{
	program = -1;
	vertexShader = -1;
	geometryShader = -1;
	pixelShader = -1;

	// Initialise texture IDs to -1 (empty)
	for (int index = 0; index < MAXTEXTURES; index++)
		textures[index] = -1;

	loadShader(vertex, GL_VERTEX_SHADER, vertexShader);
	loadShader(geometry, GL_GEOMETRY_SHADER, geometryShader);
	loadShader(pixel, GL_FRAGMENT_SHADER, pixelShader);
	linkShaders();
}


Shader::~Shader(void)
{
}

void Shader::activateShader()
{

	if (program >= 0)
	{
		glUseProgram(program);
	}

}

int	Shader::getTextureSlotID(const int textureSlot)
{
	if (textureSlot < 0 || textureSlot > MAXTEXTURES)
	{
		return -1;
	}
	else
	{
		return textures[textureSlot];
	}
}

void	Shader::setTextureSlotID(const int textureID, const int textureSlot)
{
	if (textureSlot >= 0 && textureSlot <= MAXTEXTURES)
	{
		textures[textureSlot] = textureID;
	}
	return;
}


void Shader::assignTextureToSlot(char * filename, const int texslot)
{
	if (texslot < 0 || texslot > MAXTEXTURES)
	{
		// Prepare the output in a 'string' for easier use
		std::string sLog("LOADING TEXTURE ERROR: Invalid texture slot number: ");
		sLog.append(std::to_string(texslot));
		sLog.append("\nPress OK to continue without texture.");

		// Now convert it to a wide string 'wstring' so we can pass
		// it to the windows MesageBox() function
		std::wstring wsMsgOut;
		wsMsgOut.append(sLog.begin(), sLog.end());

		MessageBox(NULL, wsMsgOut.c_str(), NULL, MB_APPLMODAL);
		return;
	}

	int currentTextureSlotValue;
	currentTextureSlotValue = getTextureSlotID(texslot);
	loadTexture(filename, currentTextureSlotValue);
	setTextureSlotID(currentTextureSlotValue, texslot);
}

void Shader::loadTexture(char * filename, int & texunit)
{
	std::string texturefullpath = "resources\\textures\\";
	texturefullpath.append(filename);

	int texwidth;
	int texheight;
	char * texdata = loadPNG(texturefullpath.c_str(), texwidth, texheight);

	if (texdata == NULL)
	{
		// Prepare the output in a 'string' for easier use
		std::string sLog("LOADING TEXTURE ERROR: ");
		sLog.append(texturefullpath);
		sLog.append("\nPress OK to continue without texture.");

		// Now convert it to a wide string 'wstring' so we can pass
		// it to the windows MesageBox() function
		std::wstring wsMsgOut;
		wsMsgOut.append(sLog.begin(), sLog.end());

		MessageBox(NULL, wsMsgOut.c_str(), NULL, MB_APPLMODAL);
		return;
	}
	
	if (texunit < 0)
	{
		texunit = textureunit;
		textureunit++;
	}

	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0 + texunit);
	GLuint tid;
	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texwidth, texheight, 0, GL_BGRA, GL_UNSIGNED_BYTE, texdata);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Alternative to enable mipmapping.
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texwidth, texheight, GL_BGRA, GL_UNSIGNED_BYTE, (const void *)texdata);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

}

void Shader::linkShaders()
{

	if (program >= 0)
	{
		glDeleteProgram(program);
	}
	program = glCreateProgram();
	if (!program)
	{
		return;
	}

	if (vertexShader >= 0)
	{
		glAttachShader(program, vertexShader);
	}
	if (geometryShader >= 0)
	{
		glAttachShader(program, geometryShader);
	}
	if (pixelShader >= 0)
	{
		glAttachShader(program, pixelShader);
	}

	glLinkProgram(program);
	int result;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (!result)
	{
		int logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char * log = new char[logLen];
		int written;
		glGetProgramInfoLog(program, logLen, &written, log);

		// Prepare the output in a 'string' for easier use
		std::string sLog("SHADER LINK ERROR: ");
		sLog.append(log);
		sLog.append("\nPress OK to continue without shader.");

		// Now convert it to a wide string 'wstring' so we can pass
		// it to the windows MesageBox() function
		std::wstring wsMsgOut;
		wsMsgOut.append(sLog.begin(), sLog.end());

		MessageBox(NULL, wsMsgOut.c_str(), NULL, MB_APPLMODAL);
		delete[] log;

		return;
	}
}


bool Shader::loadShader(char * shaderfilename, int type, int & shader)
{

	shader = glCreateShader(type);

	if (!shader)
	{
		return false;
	}
	// The shader files are found in '$(ProjectDir)\resources\shaders\'
	std::string shaderfullpath = "resources\\shaders\\";
	shaderfullpath.append(shaderfilename);

	std::ifstream shaderfile(shaderfullpath);

	if (!shaderfile.is_open())
	{
		std::string msg = "SHADER ERROR: Could not open file '";
		msg.append(shaderfullpath);
		msg.append("'");
		msg.append("\nPress OK to continue without shader.");
		std::wstring wsTmp(msg.begin(), msg.end());

		MessageBox(NULL, wsTmp.c_str(), NULL, MB_APPLMODAL);

		return false;
	}

	std::ostringstream shaderprogram;
	while (true)
	{
		char c;
		shaderfile.get(c);
		if (shaderfile.eof())
			break;
		shaderprogram << c;
	}
	std::string s = shaderprogram.str();
	const char * code = s.c_str();

	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);
	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		int logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char * log = new char[logLen];
		int written;
		glGetShaderInfoLog(shader, logLen, &written, log);

		// Prepare the output in a 'string' for easier use
		std::string sLog("SHADER COMPILE ERROR( ");
		sLog.append(shaderfilename);
		sLog.append(" ):\n");
		sLog.append(log);
		sLog.append("\nPress OK to continue without shader.");

		// Now convert it to a wide string 'wstring' so we can pass
		// it to the windows MesageBox() function
		std::wstring wsMsgOut;
		wsMsgOut.append(sLog.begin(), sLog.end());

		MessageBox(NULL, wsMsgOut.c_str(), NULL, MB_APPLMODAL);
		delete[] log;

		return false;
	}

	return true;
}

