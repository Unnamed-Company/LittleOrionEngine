#ifndef _COMPONENTAABB_H_
#define _COMPONENTAABB_H_

#include "Component.h"
#include "Mesh.h"

#include "MathGeoLib.h"
#include "GL/glew.h"

class ComponentAABB : public Component
{
public:
	enum class CollisionState
	{
		OUTSIDE,
		INSIDE,
		INTERSECT,
	};

	ComponentAABB();
	ComponentAABB(GameObject * owner);
	~ComponentAABB() = default;

	void Delete() override {};
	void GenerateBoundingBox();

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	bool IsEmpty() const;

	void ShowComponentWindow(){}

private:
	void GenerateBoundingBoxFromVertices(const std::vector<Mesh::Vertex> & vertices);
public:
	AABB bounding_box;
	AABB2D bounding_box2D;

};

#endif //_COMPONENTAABB_H_
