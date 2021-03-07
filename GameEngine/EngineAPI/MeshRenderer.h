#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <GL\glew.h>

class MeshRenderer {
public:

	GLuint texture;
	GLuint NormalTexture;
	GLuint SpecularTexture;

	std::string meshName;
	std::string textureName;
	std::string textureNormalMap;
	std::string textureSpecularMap;


	MeshRenderer();
	MeshRenderer(std::string meshName, std::string textureName, std::string textureNormalMap, std::string textureSpecularMap);

private:

	void setMesh();
	void setTexture();
};

#endif