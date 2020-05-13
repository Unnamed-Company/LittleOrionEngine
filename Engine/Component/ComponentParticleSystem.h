#ifndef _COMPONENTPARTICLESYSTEM_H
#define _COMPONENTPARTICLESYSTEM_H

#include "Component.h"
#include "MathGeoLib.h"
#include "Main/Application.h"
#include "Module/ModuleTime.h"

class GameObject;
class ComponentBillboard;

class ComponentParticleSystem : public Component
{
public:
	struct Particle {
		float3 position;
		float3 velocity;
		float4 color;
		float time_counter;
		float  life;
		bool counter = false;
		ComponentBillboard* billboard;
	};

	struct Initialization
		{
			float3 position;
			float3 rotation;
			float life;
			float3 velocity;
			float gravity;
			float size;
			float duration = 0;
		};
	ComponentParticleSystem();
	~ComponentParticleSystem() = default;

	ComponentParticleSystem(GameObject* owner);
	

	void Init();
	unsigned int FirstUnusedParticle();
	void RespawnParticle(Particle& particle);
	void Render();
	void UpdateParticle();
	float3 GetPositionOfParticle();

	void Delete() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;
	//Copy and move
	
	Component* Clone(bool original_prefab = false) const override;
	void Copy(Component* component_to_copy) const override;

public:

		
		struct ColorOverLifetime
		{
			float4 color_over_lifetime;
		};
		

	
public:
	float time_counter = 0.0F;
	float time_between_particles = 1000.0F;
	std::vector<Particle> particles;
	unsigned int max_particles = 5;
	unsigned int last_used_particle = 0;
	unsigned int nr_new_particles = 2;
};

#endif