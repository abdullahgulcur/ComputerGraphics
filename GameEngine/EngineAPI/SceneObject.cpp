#include "SceneObject.h"

SceneObject::SceneObject(std::string name) {

	Transform transform;
	this->name = name;
	this->transform = transform;

	this->ID = objectCount;
	objectCount++;
}

SceneObject::SceneObject(std::string name, Transform transform)
{
	this->name = name;
	this->transform = transform;

	this->ID = objectCount;
	objectCount++;
}

void SceneObject::setMesh(std::string meshName, std::string textureName, std::string textureNormalMap, std::string textureSpecularMap)
{
	MeshRenderer mesh(meshName, textureName, textureNormalMap, textureSpecularMap);
	this->mesh = mesh;
}
