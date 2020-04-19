#include "Prefab.h"
#include "Filesystem/Path.h"
#include "Main/Application.h"

#include "Module/ModuleAnimation.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleResourceManager.h"

#include "ResourceManagement/Manager/PrefabManager.h"
#include "ResourceManagement/Metafile/Metafile.h"

#include <algorithm>

Prefab::Prefab(Metafile* resource_metafile, std::vector<std::unique_ptr<GameObject>> && gameObjects) : Resource(resource_metafile), prefab(std::move(gameObjects))
{
}

GameObject* Prefab::Instantiate(GameObject* prefab_parent, std::unordered_map<int64_t, int64_t>* UUIDS_pairs)
{
	std::unordered_map<uint64_t, GameObject*> original_gameObject_reference;

	GameObject* parent_prefab;
	for (auto & gameObject : prefab)
	{
		GameObject* copy_in_scene = App->scene->AddGameObject(std::make_unique<GameObject>(*gameObject.get()));
		original_gameObject_reference[gameObject->UUID] = copy_in_scene;

		if (UUIDS_pairs != nullptr && UUIDS_pairs->find(gameObject->UUID) != UUIDS_pairs->end())
		{
			copy_in_scene->UUID = (*UUIDS_pairs)[gameObject->UUID];
		}

		if (gameObject->parent != nullptr && original_gameObject_reference.find(gameObject->parent->UUID) != original_gameObject_reference.end())
		{
			copy_in_scene->SetParent(original_gameObject_reference[gameObject->parent->UUID]);
		}
		else
		{
			copy_in_scene->is_prefab_parent = true;
			instances.push_back(copy_in_scene);
			parent_prefab = copy_in_scene;
		}
		copy_in_scene->prefab_reference = App->resources->Load<Prefab>(GetUUID());
		copy_in_scene->transform.Translate(float3::zero); //:D
	}
	parent_prefab->SetParent(prefab_parent);
	App->animations->UpdateAnimationMeshes();
	return parent_prefab;
}

/*
	We won't allow the user to call this function if the prefab is deleted from filesystem. In other words, there shouldn't be pointers to resources that
	doesn't exist in filesystem.
*/
void Prefab::Apply(GameObject* new_reference)
{
	*prefab.front().get() << *new_reference;
	RecursiveRewrite(prefab.front().get(), new_reference, true, false);
	for (auto old_instance : instances)
	{
		if (new_reference == old_instance)
		{
			continue;
		}
		for (auto component : new_reference->components)
		{
			component->added_by_user = false;
			component->modified_by_user = false;
		}
		*old_instance << *new_reference;
		RecursiveRewrite(old_instance, new_reference, false, false);
	}
}

void Prefab::Revert(GameObject * old_reference)
{

	for (auto component : old_reference->components)
	{
		component->added_by_user = false;
		component->modified_by_user = false;
	}
	*old_reference << *prefab.front().get();
	RecursiveRewrite(old_reference,prefab.front().get(), true, true);
}

void Prefab::RecursiveRewrite(GameObject * old_instance, GameObject * new_reference, bool original, bool revert)
{

	for (auto & child : new_reference->children)
	{
		for (auto component : child->components)
		{
			component->added_by_user = false;
			component->modified_by_user = false;
		}

		auto it = std::find_if(old_instance->children.begin(), old_instance->children.end(), [child](auto old_instance_child) {
			return child->original_UUID == old_instance_child->original_UUID &&  child->original_UUID != 0;
		});

		if (it != old_instance->children.end())
		{
			if (revert)
			{
				for (auto component : (*it)->components)
				{
					component->added_by_user = false;
					component->modified_by_user = false;
				}
			}
			**it << *child;
			RecursiveRewrite(*it, child, original, revert);
		}
		else
		{
			AddNewGameObjectToInstance(old_instance, child, original, revert);
		}

	}
	std::vector<GameObject*> gameobjects_to_remove;
	std::copy_if(
		old_instance->children.begin(),
		old_instance->children.end(),
		std::back_inserter(gameobjects_to_remove),
		[&new_reference](auto child)
	{
		return std::find_if(new_reference->children.begin(), new_reference->children.end(),
			[child](GameObject * reference_child)
		{
			return reference_child->original_UUID == child->original_UUID;
		}) == new_reference->children.end() && child->original_UUID != 0;
	}

	);
	for (auto gameobject : gameobjects_to_remove)
	{
		if (original)
		{
			RemoveGameObjectFromOriginalPrefab(gameobject);
		}
		else
		{
			App->scene->RemoveGameObject(gameobject);
		}
	}

}

void Prefab::AddNewGameObjectToInstance(GameObject * parent, GameObject * new_reference, bool original, bool revert)
{
	new_reference->original_UUID = new_reference->UUID;
	GameObject * copy;
	if (original && !revert)
	{
		prefab.emplace_back(std::make_unique<GameObject>());
		copy = prefab.back().get();
		copy->original_prefab = true;
	}
	else
	{
		copy = App->scene->CreateGameObject();
		copy->prefab_reference = App->resources->Load<Prefab>(GetUUID());
	}
	copy->SetParent(parent);
	*copy << *new_reference;
	copy->transform.SetTranslation(new_reference->transform.GetTranslation());
	for (auto new_reference_child : new_reference->children)
	{
		AddNewGameObjectToInstance(copy, new_reference_child, original, revert);
	}
}

void Prefab::RemoveGameObjectFromOriginalPrefab(GameObject * gameobject_to_remove)
{
	auto & it = std::find_if(prefab.begin(), prefab.end(), [&gameobject_to_remove](auto & gameobject) {
		return gameobject_to_remove->original_UUID == gameobject->original_UUID;
	});

	if (it != prefab.end())
	{
		for (auto child : (*it)->children)
		{
			RemoveGameObjectFromOriginalPrefab(child);
		}
		(*it)->parent->RemoveChild((*it).get());
		prefab.erase(it);
	}
}

void Prefab::RemoveInstance(GameObject * instance)
{
	auto it = std::remove_if(instances.begin(), instances.end(), [instance](auto & gameobject) {
		return instance == gameobject;
	});
	if (it != instances.end())
	{
		instances.erase(it);
	}
}

GameObject* Prefab::GetRootGameObject() const
{
	return prefab.front().get();
}

bool Prefab::IsOverwritable() const
{
	return overwritable;
}
