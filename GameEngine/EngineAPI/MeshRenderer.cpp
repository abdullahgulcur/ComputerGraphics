#include "MeshRenderer.h"
#include <Common\objloader.hpp>
#include <Common\vboindexer.hpp>
#include <Common\texture.hpp>

MeshRenderer::MeshRenderer() {

}

MeshRenderer::MeshRenderer(std::string meshName, std::string textureName, std::string textureNormalMap, std::string textureSpecularMap) {

	this->meshName = meshName;
	this->textureName = textureName;

	this->textureNormalMap = textureNormalMap;
	this->textureSpecularMap = textureSpecularMap;

	setTexture();
}

void MeshRenderer::setMesh() {


}

void MeshRenderer::setTexture() {

	const char* diffuse = textureName.c_str();
	const char* normal = textureNormalMap.c_str();
	const char* specular = textureSpecularMap.c_str();

	this->texture = loadDDS(diffuse);
	this->NormalTexture = loadBMP_custom(normal);
	this->SpecularTexture = loadDDS(specular);

}