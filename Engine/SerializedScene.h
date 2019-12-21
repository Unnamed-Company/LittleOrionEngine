#ifndef _SERIALIZEDSCENE_H_
#define _SERIALIZEDSCENE_H_

#include "Config.h"

#include <rapidjson/document.h>
#include <MathGeoLib.h>

class SerializedScene
{
public:
	SerializedScene();
	SerializedScene(const std::string& serialized_scene_string);
	~SerializedScene();
	
	void AddGameObjectsConfig(std::vector<Config*> &game_objects_config);
	void GetGameObjectsConfig(std::vector<Config*> &game_objects_config) const;

	void GetString(std::string& return_string);
	rapidjson::Document::AllocatorType& GetSceneAllocator();

private:
	rapidjson::Document scene_document;

};

#endif //_SERIALIZEDSCENE_H_
