#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <EngineAPI\SceneObject.h>

class Scene {

public:
	std::string sceneName;
	std::vector<SceneObject> sceneObjectList;
	Scene(std::string sceneName);

	void AddSceneObject(SceneObject object);
	void DeleteSceneObject(SceneObject object);
};

#endif
