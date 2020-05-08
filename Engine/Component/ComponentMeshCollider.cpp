#include "ComponentMeshCollider.h"
#include "Component/ComponentMeshRenderer.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModulePhysics.h"


ComponentMeshCollider::ComponentMeshCollider() : ComponentCollider(ComponentCollider::ColliderType::MESH)
{
	InitData();
	CreateMeshBody();
	AddBody();
}

ComponentMeshCollider::ComponentMeshCollider(GameObject* owner) : ComponentCollider(owner, ComponentCollider::ColliderType::MESH)
{
	InitData();
	ComponentMeshRenderer* mesh_renderer = static_cast<ComponentMeshRenderer*>(owner->GetComponent(ComponentType::MESH_RENDERER));
	if (mesh_renderer)
	{
		vertices.clear();
		indices.clear();
		std::shared_ptr<Mesh> mesh = mesh_renderer->mesh_to_render;
		for (auto vertex : mesh->vertices)
		{
			vertices.push_back(vertex.position.x);
			vertices.push_back(vertex.position.y);
			vertices.push_back(vertex.position.z);
		}

		indices = std::vector<int>(mesh->indices.begin(), mesh->indices.end());
		
	}
	CreateMeshBody();
	AddBody();
}

Component* ComponentMeshCollider::Clone(GameObject* owner, bool original_prefab) const
{
	ComponentMeshCollider* created_component;
	if (original_prefab)
	{
		created_component = new ComponentMeshCollider();
	}
	else
	{
		created_component = static_cast<ComponentMeshCollider*> (App->physics->CreateComponentCollider(collider_type, owner));
	}
	*created_component = *this;
	return created_component;
}

ComponentMeshCollider& ComponentMeshCollider::operator=(const ComponentMeshCollider& component_to_copy)
{
	vertices = component_to_copy.vertices;
	indices = component_to_copy.indices;
	col_shape = component_to_copy.col_shape;
	CommonAssign(component_to_copy);
	return *this;
}

void ComponentMeshCollider::UpdateDimensions()
{
	UpdateCommonDimensions();
	Scale();
}

void ComponentMeshCollider::Scale()
{
	float3 global_scale = owner->transform.GetGlobalScale();
	body->getCollisionShape()->setLocalScaling(btVector3(global_scale.x * scale.x, global_scale.y * scale.y, global_scale.z * scale.z));
}


void ComponentMeshCollider::InitData()
{
	is_attached = false;
	mass = 0.0F;
	vertices = {
			-5.0f, -5.0f, -5.0f,
			5.0f, -5.0f, -5.0f,
			5.0f,  5.0f, -5.0f,
			-5.0f,  5.0f, -5.0f,
			-5.0f, -5.0f,  5.0f,
			5.0f, -5.0f,  5.0f,
			5.0f,  5.0f,  5.0f,
			-5.0f,  5.0f,  5.0f
	};
	indices = {
		0, 1, 2,
		2, 3, 0,
		4, 5, 6,
		6, 7, 4,
		7, 3, 0,
		0, 4, 7,
		6, 2, 1,
		1, 5, 6,
		0, 1, 5,
		5, 4, 0,
		3, 2, 6,
		6, 7, 3
	};
}
void ComponentMeshCollider::CreateMeshBody()
{
	btTriangleIndexVertexArray* vertex_array = new btTriangleIndexVertexArray(indices.size() / 3, indices.data(), static_cast <int>(3 * sizeof(int)), static_cast<int>(vertices.size()), vertices.data(), static_cast <int>(3 * sizeof(float)));
	col_shape = new btBvhTriangleMeshShape(vertex_array, false);
}
