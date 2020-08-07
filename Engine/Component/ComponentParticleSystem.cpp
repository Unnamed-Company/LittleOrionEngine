#include "ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Module/ModuleEffects.h"
#include "Module/ModuleProgram.h"
#include "Module/ModuleResourceManager.h"

#include "Component/ComponentBillboard.h"
#include "GL/glew.h"

ComponentParticleSystem::ComponentParticleSystem() : Component(nullptr, ComponentType::PARTICLE_SYSTEM)
{
	
}

ComponentParticleSystem::ComponentParticleSystem(GameObject* owner) : Component(owner, ComponentType::PARTICLE_SYSTEM)
{
	
}
ComponentParticleSystem::~ComponentParticleSystem()
{
	delete billboard;
}

void ComponentParticleSystem::Init() 
{
	glGenBuffers(1, &ssbo);
	particles.reserve(MAX_PARTICLES);

	billboard = new ComponentBillboard(this->owner);
	billboard->ChangeBillboardType(ComponentBillboard::AlignmentType::WORLD);

	for (unsigned int i = 0; i < MAX_PARTICLES; ++i)
	{
		particles.emplace_back(Particle());
		particles[i].life = 0.0F;
		particles[i].particle_scale = 1.0F;
		particles[i].time_counter = particles[i].life;
		particles[i].time_passed = 0.0F;
	}

	vel_curve = BezierCurve();
}

unsigned int ComponentParticleSystem::FirstUnusedParticle()
{
	for (unsigned int i =last_used_particle; i < max_particles_number; ++i)
	{
		if (particles[i].life <= 0.0f) 
		{
			last_used_particle = i;
			return i;
		}
	}
	for (unsigned int i = 0; i < last_used_particle; ++i) 
	{
		if (particles[i].life <= 0.0f) 
		{
			last_used_particle = i;
			return i;
		}
	}
	last_used_particle = 0;
	return 0;
}

void ComponentParticleSystem::RespawnParticle(Particle& particle)
{
	particle.position_initial = float4(0.0f, 0.0f, 0.0f,0.0f);

	if (tile_random)
	{
		particle.current_sprite_x = (rand() % static_cast<int>((max_tile_value - min_tile_value) + 1) + min_tile_value);
		particle.current_sprite_y = (rand() % static_cast<int>((max_tile_value - min_tile_value) + 1) + min_tile_value);
	}

	if (change_size)
	{
		particle.size = float2(min_size_of_particle);
	}
	else if (size_random)
	{
		float size = min_size_of_particle + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max_size_of_particle - min_size_of_particle)));
		particle.size = float2(size);
	}
	else
	{
		particle.size = particles_size;
	}

	switch (type_of_particle_system)
	{
		case SPHERE:
			particle.velocity_initial = float4(float3::RandomDir(LCG(), 1), 0.0f);
		break;
		case BOX:
		{
			float random_x = (rand() % ((max_range_random_x - min_range_random_x) + 1) + min_range_random_x) / 100.f;

			float random_z = (rand() % ((max_range_random_z - min_range_random_z) + 1) + min_range_random_z) / 100.f;

			particle.velocity_initial = float4(0, 1, 0, 0);
			if (enabled_random_x)
			{
				particle.position_initial.x = random_x;
			}
			else
			{
				particle.position_initial.x = position_x / 100.0F;
			}
			if (enabled_random_z)
			{
				particle.position_initial.z = random_z;
			}
			else
			{
				particle.position_initial.z = position_z / 100.0F;
			}
		
		break;
		}
		case CONE:
			float angle = ((rand() % 100)/100.f) * 2 * math::pi;
			float radius = inner_radius * sqrt((rand() % 100) / 100.f);
			float proportion = outer_radius / inner_radius;
			particle.position_initial.x = radius * math::Cos(angle);
			particle.position_initial.z = radius * math::Sin(angle);
			float distance = velocity_particles_start * particles_life_time;
			float height = sqrt((distance*distance) - ((radius*proportion) - radius)*((radius*proportion) - radius));
			float4 final_local_position = float4(particle.position_initial.x*proportion, height, particle.position_initial.z*proportion,0.0f);
			particle.velocity_initial = (final_local_position - particle.position_initial);
		break;
	}

	particle.velocity_initial.Normalize3();
	
	particle.color = initial_color;
	particle.life = particles_life_time * 1000;
	particle.time_counter = particle.life;
	particle.time_passed = 0.0F;

	particle.geometric_space = float4x4::FromTRS(owner->transform.GetGlobalTranslation(), owner->transform.GetRotation(), float3::one);
	particle.inital_random_orbit = rand();
	particle.random_velocity_percentage = (float)rand() / RAND_MAX;
}

void ComponentParticleSystem::Render()
{

	BROFILER_CATEGORY("Particle Render", Profiler::Color::OrangeRed);

	if (active && playing && billboard->billboard_texture != nullptr && billboard->billboard_texture->initialized)
	{
		unsigned int variation = GetParticlesSystemVariation();
		shader_program = App->program->UseProgram("Particles", variation);

		billboard->CommonUniforms(shader_program);
		static_assert(sizeof(Particle) % (sizeof(float) * 4) == 0); //Check comment on particle struct
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(Particle))*(num_of_alive_particles), particles.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glBindVertexArray(billboard->vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particles.size());
		glBindVertexArray(0);


		glUseProgram(0);
	}
	
}

void ComponentParticleSystem::Update()
{
	if (active && playing)
	{
		time_counter += App->time->real_time_delta_time;

		if (time_counter >= (time_between_particles * 1000))
		{
			if (loop)
			{
				int unused_particle = FirstUnusedParticle();
				RespawnParticle(particles[unused_particle]);
				time_counter = 0.0F;
			}
		}

		CalculateGravityVector();

		// update all particles
		num_of_alive_particles = 0;
		for (unsigned int i = 0; i < playing_particles_number; ++i)
		{
			Particle& p = particles[i];

			if (p.life > 0.0f)
			{
				UpdateParticle(p);
				std::iter_swap(particles.begin() + i, particles.begin() + num_of_alive_particles);
				++num_of_alive_particles;
			}
			else if(emitting)
			{
				++number_emited;
			}
		}
		if (emitting && number_emited >= playing_particles_number)
		{
			Stop();
		}
	}
}

void ComponentParticleSystem::UpdateParticle(Particle& particle)
{
	particle.life -= App->time->real_time_delta_time; // reduce life
	particle.time_passed += App->time->real_time_delta_time;

	//velocity over time
	float4 velocity = particle.velocity_initial * velocity_particles_start * velocity_factor_mod;
	float4 vel_curve_interpolated = float4::zero;
	float4 acceleration = gravity_vector;
	if (velocity_over_time)
	{
		switch (type_of_velocity_over_time)
		{
		case CONSTANT:
			velocity *= velocity_over_time_speed_modifier;
			break;
		case LINEAR:
		{
			velocity *= velocity_over_time_speed_modifier;
			float accel = (velocity_over_time_speed_modifier_second - velocity_over_time_speed_modifier) * velocity_factor_mod / particles_life_time / 1000;
			acceleration += particle.velocity_initial * accel;
			break;
		}
		case RANDOM_BETWEEN_TWO_CONSTANTS:
			velocity *= velocity_over_time_speed_modifier + (velocity_over_time_speed_modifier_second - velocity_over_time_speed_modifier) * particle.random_velocity_percentage;
			break;
		case CURVE:
			//velocity *= velocity_over_time_speed_modifier;
			//float percentage = ImGui::BezierValue(particle.time_passed / particles_life_time / 1000, velocity_bezier_curve);
			//float vel_perc = (velocity_over_time_speed_modifier_second - velocity_over_time_speed_modifier) * percentage;
			//vel_curve_interpolated = particle.velocity_initial * vel_perc * velocity_factor_mod;

			break;
		}
	}

	//update position
	particle.position = particle.position_initial + ((velocity + vel_curve_interpolated ) * particle.time_passed) +
		(acceleration * particle.time_passed * particle.time_passed / 2);

	if (orbit)
	{
		particle.position.z += sin((particle.time_passed *0.001f) + particle.inital_random_orbit);
		particle.position.x += cos((particle.time_passed *0.001f) + particle.inital_random_orbit);
	}
	//alpha fade
	if (fade)
	{
		float progress = particle.time_passed * 0.001f / fade_time;
		particle.color.w = math::Lerp(0.f, 1.f, 1 - progress);
	}

	//fade color
	if (fade_between_colors)
	{
		float progress = particle.time_passed * 0.001f / color_fade_time;
		float3 tmp_color = float3::Lerp(initial_color.xyz(), color_to_fade.xyz(), progress);
		particle.color.x = tmp_color.x;
		particle.color.y = tmp_color.y;
		particle.color.z = tmp_color.z;
	}

	//size fade
	if (change_size)
	{
		float progress = particle.time_passed * 0.001f / size_change_speed;
		particle.size = float2::Lerp(float2(min_size_of_particle), float2(max_size_of_particle), progress);
	}

	//animation 
	if (billboard->is_spritesheet && !tile_random)
	{
		float progress = particle.time_passed * 0.001f / particles_life_time;
		billboard->ComputeAnimationFrame(progress);

		particle.current_sprite_x = billboard->current_sprite_x;
		particle.current_sprite_y = billboard->current_sprite_y;
	}

	if (follow_owner)
	{
		particle.geometric_space = float4x4::FromTRS(owner->transform.GetGlobalTranslation(), owner->transform.GetGlobalRotation(), float3::one);
	}
	particle.model = particle.geometric_space * float4x4::FromTRS(particle.position.xyz(), Quat::identity, float3(particle.size, 1.f));
	particle.model = particle.model.Transposed();
}

void ComponentParticleSystem::SetParticleTexture(uint32_t texture_uuid)
{
	this->texture_uuid = texture_uuid;
	billboard->ChangeTexture(texture_uuid);
}
void ComponentParticleSystem::Delete()
{
	App->effects->RemoveComponentParticleSystem(this);
}

unsigned int ComponentParticleSystem::GetParticlesSystemVariation()
{
	unsigned int variation = 0;

	switch (billboard->alignment_type)
	{
	case ComponentBillboard::AlignmentType::VIEW_POINT:
		variation |= static_cast<unsigned int>(ModuleProgram::ShaderVariation::ENABLE_BILLBOARD_VIEWPOINT_ALIGNMENT);
		break;

	case ComponentBillboard::AlignmentType::AXIAL:
		variation |= static_cast<unsigned int>(ModuleProgram::ShaderVariation::ENABLE_BILLBOARD_AXIAL_ALIGNMENT);
		break;
	}

	if (billboard->is_spritesheet)
	{
		variation |= static_cast<unsigned int>(ModuleProgram::ShaderVariation::ENABLE_SPRITESHEET);
	}

	return variation;
}

void ComponentParticleSystem::SpecializedSave(Config& config) const
{

	billboard->SpecializedSave(config);
	config.AddInt(static_cast<int>(type_of_particle_system), "Type of particle system");
	config.AddBool(loop, "Loop");
	config.AddBool(active, "Active");
	config.AddFloat(min_size_of_particle, "Max Size Particles");
	config.AddFloat(max_size_of_particle, "Min Size Particles");
	config.AddFloat2(particles_size, "Particle Size");
	config.AddBool(size_random, "Size random");
	config.AddBool(change_size, "Change size");
	config.AddBool(tile_random, "Tile random");
	config.AddFloat(max_tile_value, "Max Tile");
	config.AddFloat(min_tile_value, "Min Tile");

	config.AddFloat(velocity_particles_start, "Velocity of particles");
	config.AddFloat(gravity_modifier, "Gravity modifier");

	config.AddFloat(time_between_particles, "Time Between Particles");
	config.AddFloat(particles_life_time, "Particles Life Time");

	config.AddBool(follow_owner, "Follow owner");

	config.AddBool(enabled_random_x, "Random X position");
	config.AddInt(max_range_random_x, "Max range position x");
	config.AddInt(min_range_random_x, "Min range position x");
	config.AddInt(position_x, "Position X");

	config.AddBool(enabled_random_z, "Random Z position");
	config.AddInt(max_range_random_z, "Max range position z");
	config.AddInt(min_range_random_z, "Min range position z");
	config.AddInt(position_z, "Position Z");

	config.AddInt(max_particles_number, "Max particles");

	config.AddFloat(inner_radius, "Inner Radius");
	config.AddFloat(outer_radius, "Outer Radius");

	config.AddColor(initial_color, "Intial Color");
	config.AddBool(fade, "Fade");
	config.AddFloat(fade_time, "Fade Time");
	config.AddFloat(size_change_speed, "Size Fade Time");
	config.AddFloat(color_fade_time, "Color Fade Time");
	config.AddBool(fade_between_colors, "Fade between Colors");
	config.AddColor(color_to_fade, "Color to fade");
	config.AddBool(orbit, "Orbit");

	config.AddBool(velocity_over_time, "Velocity Over Time");
	config.AddInt(type_of_velocity_over_time, "Type of Velocity");
	config.AddFloat(velocity_over_time_speed_modifier, "Velocity Initial");
	config.AddFloat(velocity_over_time_speed_modifier_second, "Velocity Final");
}

void ComponentParticleSystem::SpecializedLoad(const Config& config)
{
	billboard->SpecializedLoad(config);
	type_of_particle_system = static_cast<TypeOfParticleSystem>(config.GetInt("Type of particle system", static_cast<int>(TypeOfParticleSystem::BOX)));
	
	loop = config.GetBool("Loop", true);
	min_size_of_particle = config.GetFloat("Max Size Particles", 0.2);
	max_size_of_particle = config.GetFloat("Min Size Particles", 0.2);
	config.GetFloat2("Particle Size", particles_size, float2(0.2f));
	size_random = config.GetBool("Size random", false);
	tile_random = config.GetBool("Tile random", false);
	change_size = config.GetBool("Change size", false);
	max_tile_value = config.GetFloat("Max Tile", 0);
	min_tile_value = config.GetFloat("Min Tile", 4);

	velocity_particles_start = config.GetFloat("Velocity of particles", 1.0F);
	gravity_modifier = config.GetFloat("Gravity modifier", 0.f);

	time_between_particles = config.GetFloat("Time Between Particles", 0.2F);
	particles_life_time = config.GetFloat("Particles Life Time", 3.0F);

	follow_owner = config.GetBool("Follow owner", false);

	enabled_random_x = config.GetBool("Random X position", true);
	max_range_random_x = config.GetInt("Max range position x", 100);
	min_range_random_x = config.GetInt("Min range position x", -100);
	position_x = config.GetInt("Position X", 0);

	enabled_random_z = config.GetBool("Random Z position", true);
	max_range_random_z = config.GetInt("Max range position z", 100);
	min_range_random_z = config.GetInt("Min range position z",-100);
	position_z = config.GetInt("Position Z", 0);

	max_particles_number = config.GetInt("Max particles", MAX_PARTICLES);
	playing_particles_number = max_particles_number;
	inner_radius = config.GetFloat("Inner Radius", 1.0F);
	outer_radius = config.GetFloat("Outer Radius", 3.0F);

	float4 color_aux;
	config.GetColor("Intial Color", initial_color, float4::one);

	fade = config.GetBool("Fade", false);
	fade_time = config.GetFloat("Fade Time", 1.0F);
	size_change_speed = config.GetFloat("Fade Time", 1.0F);
	color_fade_time = config.GetFloat("Color Fade Time", 1.0F);

	fade_between_colors = config.GetBool("Fade between Colors", false);
	config.GetColor("Color to fade", color_to_fade, float4::one);

	orbit = config.GetBool("Orbit", false);

	velocity_over_time = config.GetBool("Velocity Over Time", false);
	type_of_velocity_over_time = static_cast<TypeOfVelocityOverTime>(config.GetInt("Type of Velocity", CONSTANT));
	velocity_over_time_speed_modifier = config.GetFloat("Velocity Initial", 1.0F);
	velocity_over_time_speed_modifier_second = config.GetFloat("Velocity Final", 2.0F);
}

Component* ComponentParticleSystem::Clone(bool original_prefab) const
{

	ComponentParticleSystem* created_component;
	if (original_prefab)
	{
		created_component = new ComponentParticleSystem();
	}
	else
	{
		created_component = App->effects->CreateComponentParticleSystem();
	}

	created_component->Init();
	auto original_billboard = created_component->billboard;
	*created_component = *this;
	*original_billboard = *this->billboard;
	created_component->billboard = original_billboard;
	CloneBase(static_cast<Component*>(created_component));
	return created_component;
};

void ComponentParticleSystem::Copy(Component * component_to_copy) const
{
	*component_to_copy = *this;
	ComponentParticleSystem* component_particle_system = static_cast<ComponentParticleSystem*>(component_to_copy);
	auto original_billboard = component_particle_system->billboard;
	*component_particle_system = *this;
	*original_billboard = *this->billboard;
	component_particle_system->billboard = original_billboard;
}

void ComponentParticleSystem::Emit(size_t count)
{
	playing = true;
	emitting = true;
	number_emited = 0;
	if (count < max_particles_number)
	{
		playing_particles_number = count;
		for (size_t i = 0; i < playing_particles_number; i++)
		{
			RespawnParticle(particles[i]);
		}
	}

}

void ComponentParticleSystem::Play()
{
	playing = true;
	emitting = false;
	playing_particles_number = MAX_PARTICLES;
}

void ComponentParticleSystem::Stop()
{
	time_counter = 0.0F;
	playing_particles_number = max_particles_number;
	emitting = false;
	for (unsigned int i = 0; i < max_particles_number; i++)
	{

		particles[i].life = 0.0F;
		particles[i].particle_scale = 1.0F;
		particles[i].time_counter = particles[i].life;
		particles[i].time_passed = 0.0F;
	}
	playing = false;
}

ENGINE_API void ComponentParticleSystem::Pause()
{
	playing = false;
	emitting = false;
}

void ComponentParticleSystem::OrbitX(float angle, Particle& particle)
{
	float3 focus_vector = owner->transform.GetTranslation() - owner->transform.GetTranslation();

	const float adjusted_angle = App->time->real_time_delta_time  * -angle;
	Quat rotation = Quat::RotateY(adjusted_angle);

	focus_vector = rotation * focus_vector;
	auto position = focus_vector + owner->transform.GetTranslation();
	particle.position.x = position.x;
	particle.position.z = position.z;
}

void ComponentParticleSystem::CalculateGravityVector()
{
	float4x4 gravity_rotation = float4x4::FromTRS(float3::one, owner->transform.GetGlobalRotation(), float3::one);
	gravity_rotation.Transpose();
	gravity_vector = gravity_rotation * float4(0, gravity_modifier * gravity_factor_mod, 0, 0);
}

void ComponentParticleSystem::Disable()
{
	active = false;
	Stop();
}

void ComponentParticleSystem::Enable()
{
	active = true;
}