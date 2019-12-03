#include "ComponentAABB.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "Application.h"

#include "Module/ModuleRender.h"

ComponentAABB::ComponentAABB() : Component(nullptr, ComponentType::MATERIAL)
{

}

ComponentAABB::ComponentAABB(GameObject * owner) : Component(owner, ComponentType::AABB)
{

}

ComponentAABB::~ComponentAABB()
{

}

void ComponentAABB::Enable()
{
	active = true;
}

void ComponentAABB::Disable()
{
	active = false;
}

void ComponentAABB::Update()
{

}

void ComponentAABB::GenerateBoundingBox()
{
	bool has_mesh = false;
	ComponentMesh * owner_mesh = static_cast<ComponentMesh*>(owner->GetComponent(ComponentType::MESH));
	has_mesh = owner_mesh != nullptr;
	
	if (has_mesh)
	{
		GenerateBoundingBoxFromVertices(owner_mesh->vertices);
	}
	else
	{
		bounding_box = AABB(float3::zero, float3::zero);
	}
	
	bounding_box.TransformAsAABB(owner->transform.GetGlobalModelMatrix());
}

void ComponentAABB::GenerateBoundingBoxFromVertices(const std::vector<ComponentMesh::Vertex> & vertices)
{
	bounding_box.SetNegativeInfinity();
	for (unsigned int i = 0; i < vertices.size(); ++i)
	{
		bounding_box.Enclose(vertices[i].position);
	}
}

bool ComponentAABB::IsEmpty() const
{
	return bounding_box.Size().Length() == 0;
}

std::vector<float> ComponentAABB::GetVertices() const
{
	float3 tmp_vertices[8];
	bounding_box.GetCornerPoints(&tmp_vertices[0]);

	std::vector<float> vertices(24);
	for (unsigned int i = 0; i < 8; ++i)
	{
		vertices[i * 3] = tmp_vertices[i].x;
		vertices[i * 3 + 1] = tmp_vertices[i].y;
		vertices[i * 3 + 2] = tmp_vertices[i].z;
	}

	return vertices;
}