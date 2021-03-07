#include "Scene.h"

Scene::Scene(std::string sceneName) {
	this->sceneName = sceneName;
}

void Scene::AddSceneObject(SceneObject object)
{
	sceneObjectList.push_back(object);
}

void Scene::DeleteSceneObject(SceneObject object)
{
}
