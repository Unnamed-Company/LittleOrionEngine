#include "ModulePhysics.h"
#include "Component/ComponentBoxCollider.h"
#include "Component/ComponentCapsuleCollider.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentCylinderCollider.h"
#include "Component/ComponentMeshCollider.h"
#include "Component/ComponentSphereCollider.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "ModuleTime.h"
#include "ModuleDebugDraw.h"
#include <GL/glew.h>
#include "EditorUI/DebugDraw.h"


ModulePhysics::ModulePhysics()
{
	physics_timer = new Timer();	
}

ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Init()
{
	physics_timer->Start();
	
	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	debug_draw = new DebugDrawer();
	debug_draw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	world->setDebugDrawer(debug_draw);
	
	return true;
}

update_status ModulePhysics::PreUpdate()
{
	
	return update_status::UPDATE_CONTINUE;
}

update_status ModulePhysics::Update()
{
	ms = physics_timer->Read();
		
	//update the world
	world->stepSimulation(App->time->delta_time, 2);

	//float3 distance = gravity * App->time->delta_time * App->time->delta_time;

	if (show_physics)
	{
		world->debugDrawWorld();
	}
		
	for (auto collider : colliders)
	{
		if (App->time->isGameRunning() && collider->active_physics && collider->IsEnabled()
			&& collider->collider_type != ComponentCollider::ColliderType::MESH)
		{
			collider->MoveBody();
		}
		else
		{	
			if (App->time->isGameRunning()) {
				
				if (collider->manual_movement)
				{
					// fake gravity
					float3 last_position = collider->owner->transform.GetGlobalTranslation();
					if (!collider->is_static)
					{
		
						collider->owner->transform.SetGlobalMatrixTranslation(last_position + gravity);
						collider->UpdateDimensions();
						
						if (collider->DetectCollision())
						{
							collider->owner->transform.SetGlobalMatrixTranslation(last_position);
							collider->UpdateDimensions();
						}
					}
				}
				else {
					collider->UpdateDimensions();
				}
			
			}
			else {
				collider->UpdateDimensions();
			}
					
		}

		if(collider->collider_type != ComponentCollider::ColliderType::MESH)
		{
			world->synchronizeSingleMotionState(collider->body);
		}
	}
	
	float ms2;
	ms2 = physics_timer->Read();

	if (ms_info.size() < 100)
	{
		ms_info.push_back(ms2 - ms);
	}
	else
	{
		rotate(ms_info.begin(), ms_info.begin() + 1, ms_info.end());
		ms_info[99] = ms2 - ms;
	}
	
	return update_status::UPDATE_CONTINUE;
}

void ModulePhysics::UpdateAllDimensions()
{
	for (auto collider : colliders)
	{
		collider->UpdateDimensions();
		

		if (collider->collider_type != ComponentCollider::ColliderType::MESH)
		{
			world->synchronizeSingleMotionState(collider->body);
		}
	}
}

bool ModulePhysics::CleanUp()
{
	return true;
}

void ModulePhysics::SetGravity(float3& newGravity)
{
	world->setGravity(btVector3(newGravity.x, newGravity.y, newGravity.z));
}

float3 ModulePhysics::GetGravity() const
{
	return gravity;
}

ComponentCollider* ModulePhysics::CreateComponentCollider(ComponentCollider::ColliderType collider_type, GameObject* owner)
{
	ComponentCollider* new_collider = nullptr;
	switch (collider_type)
	{
		case ComponentCollider::ColliderType::BOX:
			new_collider = new ComponentBoxCollider(owner);
			break;
		case ComponentCollider::ColliderType::CAPSULE:
			new_collider = new ComponentCapsuleCollider(owner);
			break;
		case ComponentCollider::ColliderType::SPHERE:
			new_collider = new ComponentSphereCollider(owner);
			break;
		case ComponentCollider::ColliderType::CYLINDER:
			new_collider = new ComponentCylinderCollider(owner);
			break;
		case ComponentCollider::ColliderType::MESH:
			new_collider = new ComponentMeshCollider(owner);
			break;
	}
	if (new_collider)
	{
		colliders.push_back(new_collider);
	}
	return new_collider;
}

void ModulePhysics::RemoveComponentCollider(ComponentCollider* collider_to_remove)
{
	auto it = std::find(colliders.begin(), colliders.end(), collider_to_remove);
	if (it != colliders.end())
	{
		delete *it;
		colliders.erase(it);
	}
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	
	dd::line(float3(from.getX(), from.getY(), from.getZ()), float3(to.getX(), to.getY(), to.getZ()), float3(1.0f, 0.0f, 0.0f));
	
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = btIDebugDraw::DebugDrawModes::DBG_DrawWireframe;
}

int DebugDrawer::getDebugMode() const
{
	return btIDebugDraw::DBG_DrawWireframe;
}

bool ModulePhysics::RaycastWorld(const btVector3 &Start, btVector3 &End, btVector3 &Normal)
{

	btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);
	//btCollisionWorld::RayResultCallback RayCallback(Start, End);
	//Magic Line for not jumping on enemies
	/*RayCallback.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;*/
	
	world->rayTest(Start, End, RayCallback);
	if (RayCallback.hasHit())
	{
		End = RayCallback.m_hitPointWorld;
		Normal = RayCallback.m_hitNormalWorld;
		return true;
	}

	return false;
}

int ModulePhysics::GetRaycastWorldId(const btVector3& start, btVector3& end, btVector3& normal)
{
	btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);

	world->rayTest(start, end, RayCallback);
	if (RayCallback.hasHit())
	{
		end = RayCallback.m_hitPointWorld;
		normal = RayCallback.m_hitNormalWorld;
		return RayCallback.m_collisionObject->getWorldArrayIndex();
	}

	return -1;
}
