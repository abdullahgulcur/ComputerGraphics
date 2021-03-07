#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <EngineAPI\Transform.h>
#include <string>
#include <EngineAPI\MeshRenderer.h>

class SceneObject : Transform{

public:

	static int objectCount;
	int ID;

	std::string name;
	Transform transform;
	MeshRenderer mesh;

	SceneObject(std::string name);
	SceneObject(std::string name, Transform transform);
	void setMesh(std::string meshName, std::string textureName, std::string textureNormalMap, std::string textureSpecularMap);

};

#endif
