#ifndef _PREFABMANAGER_H_
#define _PREFABMANAGER_H_

#include "Importer.h"

class GameObject;
class PrefabManager
{
public:
	PrefabManager() = default;
	~PrefabManager() = default;

	void Save(const std::string &path, const GameObject * gameobject_to_save) const;
	void Load(const std::string &path) const;
};

#endif