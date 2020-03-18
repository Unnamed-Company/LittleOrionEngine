#include "GameObject.h"
#include "Application.h"
#include "Helper/Config.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleProgram.h"
#include "Module/ModuleLight.h"
#include "Module/ModuleRender.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTexture.h"
#include "Module/ModuleUI.h"
#include "ResourceManagement/Resources/Texture.h"
#include "UI/Panel/PanelHierarchy.h"

#include "Component/Component.h"
#include "Component/ComponentCamera.h"
#include "Component/ComponentMaterial.h"
#include "Component/ComponentMesh.h"
#include "Component/ComponentLight.h"
#include "Component/ComponentCanvas.h"
#include "Component/ComponentUI.h"

#include "Brofiler/Brofiler.h"
#include <pcg_basic.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <algorithm>

GameObject::GameObject() : aabb(this), UUID(pcg32_random())
{
	CreateTransform();
}

GameObject::GameObject(unsigned int UUID) : aabb(this),  UUID(UUID)
{
	CreateTransform();
}

GameObject::GameObject(const std::string name) :
	name(name),
	aabb(this),
	UUID(pcg32_random())
{
	CreateTransform();
}

void GameObject::Delete(std::vector<GameObject*> & children_to_remove)
{
	children_to_remove.push_back(this);
	if(!is_static)
		App->renderer->RemoveAABBTree(this);
	if (parent != nullptr)
	{
		parent->RemoveChild(this);
	}

	for (int i = (components.size() - 1); i >= 0; --i)
	{
		components[i]->Delete();
		components[i] = nullptr;
	}
	for (int i = (children.size() - 1); i >= 0; --i)
	{
		children[i]->parent = nullptr;
		children[i]->Delete(children_to_remove);
	}
}
bool GameObject::IsEnabled() const
{
	return active;
}

void GameObject::SetEnabled(bool able)
{
	active = able;
	
	for(auto component : components)
	{
		(able) ? component->Enable() : component->Disable();
	}

	for(auto child : children)
	{
		child->SetEnabled(able);
	}
}

void GameObject::SetStatic(bool is_static)
{
	SetHierarchyStatic(is_static);
	App->renderer->GenerateQuadTree(); // TODO: Check this. This could be called with ungenerated bounding boxes, resulting in a wrong quadtree.
}

void GameObject::SetHierarchyStatic(bool is_static)
{
	this->is_static = is_static;

	//AABBTree
	(is_static) ? App->renderer->RemoveAABBTree(this) : App->renderer->InsertAABBTree(this);
	
	for (auto & child : children)
	{
		child->SetStatic(is_static);
	}
}

Config GameObject::SaveTransform() const
{
	Config config;
	ComponentTransform* transform = GetTransform();
	if(transform != nullptr) transform->Save(config);

	ComponentTransform2D* transform_2d = GetTransform2D();
	if (transform_2d != nullptr) transform_2d->Save(config);

	return config;
}

void GameObject::LoadTransform(Config config) const
{
	Config transform_config;
	config.GetChildConfig("Transform", transform_config);

	std::string transform_type;
	config.GetString("TransformType", transform_type, "3D");

	if (transform_type == "2D")
	{
		GetTransform2D()->Load(transform_config);
	}
	else if(transform_type == "3D")
	{
		GetTransform()->Load(transform_config);

	}
}

void GameObject::CreateTransform()
{
	if (IsChildOfUI())
	{
		transform = new ComponentTransform2D();
		transform->owner = this;
		((ComponentTransform2D*)transform)->Translate(float2::zero);//trigger transform change
	}
	else
	{
		transform = new ComponentTransform();
		transform->owner = this;
		((ComponentTransform*)transform)->Translate(float3::zero);//trigger transform change

	}

}

bool GameObject::IsChildOfUI()
{
	return parent != nullptr && (parent->GetComponent(Component::ComponentType::CANVAS) != nullptr || parent->GetTransform2D() != nullptr);
}

bool GameObject::IsStatic() const
{
	return is_static;
}

bool GameObject::IsVisible(const ComponentCamera & camera) const
{
	ComponentMesh* mesh = static_cast<ComponentMesh*>(GetComponent(Component::ComponentType::MESH));
	if ((mesh != nullptr && !mesh->IsEnabled()) || !IsEnabled() || !camera.IsInsideFrustum(aabb.bounding_box))
	{
		return false;
	}
	return true;
}
void GameObject::Update()
{
	BROFILER_CATEGORY("GameObject Update", Profiler::Color::Green);

	for (unsigned int i = 0; i < components.size(); ++i)
	{
		components[i]->Update();
	}
}

void GameObject::Save(Config& config) const
{
	config.AddUInt(UUID, "UUID");
	if (parent != nullptr)
	{
		config.AddUInt(parent->UUID, "ParentUUID");
	}
	config.AddString(name, "Name");

	config.AddBool(is_static, "IsStatic");
	config.AddBool(active, "Active");

	Config transform_config = SaveTransform();
	config.AddChildConfig(transform_config, "Transform");

	std::vector<Config> gameobject_components_config(components.size());
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		components[i]->Save(gameobject_components_config[i]);
	}
	config.AddChildrenConfig(gameobject_components_config, "Components");
}

void GameObject::Load(const Config& config)
{
	UUID = config.GetUInt("UUID", 0);
	assert(UUID != 0);

	config.GetString("Name", name, "GameObject");

	uint64_t parent_UUID = config.GetUInt("ParentUUID", 0);
	GameObject* game_object_parent = App->scene->GetGameObject(parent_UUID); 
	assert(game_object_parent != nullptr);
	if (parent_UUID != 0)
	{
		game_object_parent->AddChild(this); //TODO: This should be in scene. Probably D:
	}

	SetStatic(config.GetBool("IsStatic", false));
	active = config.GetBool("Active", true);

	LoadTransform(config);

	std::vector<Config> gameobject_components_config;
	config.GetChildrenConfig("Components", gameobject_components_config);
	for (unsigned int i = 0; i < gameobject_components_config.size(); ++i)
	{
		uint64_t component_type_uint = gameobject_components_config[i].GetUInt("ComponentType", 0);
		assert(component_type_uint != 0);
		
		Component::ComponentType component_type = Component::GetComponentType(static_cast<unsigned int>(component_type_uint));
		Component* created_component = CreateComponent(component_type);
		created_component->Load(gameobject_components_config[i]);
	}
}

void GameObject::SetParent(GameObject *new_parent)
{
	if (new_parent == parent)
	{
		return;
	}

	if (parent != nullptr) 
	{
		parent->RemoveChild(this);
	}

	//Change the transform to 2D if child of ui
	if (IsChildOfUI())
	{
		RemoveComponent(GetTransform());
		CreateComponent(Component::ComponentType::TRANSFORM2D);
	}
	new_parent->AddChild(this);
}

void GameObject::AddChild(GameObject *child)
{
	if (child->parent != nullptr)
	{
		child->parent->RemoveChild(child);
	}
	
	child->parent = this;
	child->UpdateHierarchyDepth();
	child->UpdateHierarchyBranch();

	child->GetTransform()->ChangeLocalSpace(GetTransform()->GetGlobalModelMatrix());
	children.push_back(child);
}

void GameObject::RemoveChild(GameObject *child)
{
	std::vector<GameObject*>::iterator found = std::find(children.begin(), children.end(), child);
	if (found == children.end())
	{
		APP_LOG_ERROR("Incosistent GameObject Tree.");
	}
	children.erase(found);
	child->parent = nullptr;
	child->hierarchy_depth = 0;
}


Component* GameObject::CreateComponent(const Component::ComponentType type)
{
	Component *created_component;
	switch (type)
	{
	case Component::ComponentType::CAMERA:
		created_component = App->cameras->CreateComponentCamera();
		break;

	case Component::ComponentType::MATERIAL:
		created_component = App->texture->CreateComponentMaterial();
		break;

	case Component::ComponentType::MESH:
		created_component = App->renderer->CreateComponentMesh();
		break;

	case Component::ComponentType::LIGHT:
		created_component = App->lights->CreateComponentLight();
		break;
	case Component::ComponentType::CANVAS:
		created_component = App->ui->CreateComponentCanvas();
		break;
	case Component::ComponentType::UI:
		created_component = App->ui->CreateComponentUI();
		break;
	default:
		APP_LOG_ERROR("Error creating component. Incorrect component type.");
		return nullptr;
	}

	created_component->owner = this;
	components.push_back(created_component);
	return created_component;
}

void GameObject::RemoveComponent(Component * component_to_remove) 
{
	auto it = std::find(components.begin(), components.end(), component_to_remove);
	if (it != components.end()) 
	{
		component_to_remove->Delete();
		components.erase(it);
	}
}

Component* GameObject::GetComponent(const Component::ComponentType type) const
{
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		if (components[i]->GetType() == type)
		{
			return components[i];
		}
	}
	return nullptr;
}

ComponentTransform * GameObject::GetTransform() const
{
	if (transform != nullptr) return (ComponentTransform*)transform;

	return (ComponentTransform*)GetComponent(Component::ComponentType::TRANSFORM);
}

ComponentTransform2D * GameObject::GetTransform2D() const
{
	if (transform != nullptr) return (ComponentTransform2D*)transform;

	return (ComponentTransform2D*) GetComponent(Component::ComponentType::TRANSFORM2D);
}

void GameObject::MoveUpInHierarchy() const
{
	std::vector<GameObject*>::iterator silbings_position = std::find(parent->children.begin(), parent->children.end(), this);
	if (silbings_position == parent->children.end())
	{
		APP_LOG_ERROR("Incosistent GameObject Tree.");
	}

	std::vector<GameObject*>::iterator swapped_silbing = max(silbings_position - 1, parent->children.begin());
	std::iter_swap(silbings_position, swapped_silbing);
}

void GameObject::MoveDownInHierarchy() const
{
	std::vector<GameObject*>::iterator silbings_position = std::find(parent->children.begin(), parent->children.end(), this);
	if (silbings_position == parent->children.end())
	{
		APP_LOG_ERROR("Incosistent GameObject Tree.");
	}

	std::vector<GameObject*>::iterator swapped_silbing = min(silbings_position + 1, parent->children.end() - 1);
	std::iter_swap(silbings_position, swapped_silbing);
}

bool GameObject::IsAboveInHierarchy(const GameObject &potential_child) const
{
	return (
		this->hierarchy_branch == potential_child.hierarchy_branch
		&& this->hierarchy_depth < potential_child.hierarchy_depth
	);
}

void GameObject::UpdateHierarchyDepth()
{
	hierarchy_depth = parent->hierarchy_depth + 1;
	for (unsigned int i = 0; i < children.size(); ++i)
	{
		children[i]->UpdateHierarchyDepth();
	}
}

void GameObject::UpdateHierarchyBranch()
{
	if (parent->hierarchy_branch == 0) // PARENT IS ROOT GAMEOBJECT
	{
		hierarchy_branch = App->editor->hierarchy->GetNextBranch();
	}
	else
	{
		hierarchy_branch = parent->hierarchy_branch;
	}

	for (unsigned int i = 0; i < children.size(); ++i)
	{
		children[i]->UpdateHierarchyBranch();
	}
}

void GameObject::RenderMaterialTexture(unsigned int shader_program) const
{
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		if (components[i]->GetType() == Component::ComponentType::MATERIAL)
		{
			ComponentMaterial* current_material = (ComponentMaterial*)components[i];
			current_material->Render(shader_program);
		}
	}
}

int GameObject::GetHierarchyDepth() const
{
	return hierarchy_depth;
}

void GameObject::SetHierarchyDepth(int value)
{
	hierarchy_depth = value;
}