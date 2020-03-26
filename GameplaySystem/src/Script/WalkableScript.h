#ifndef  __WALKABLESCRIPT_H__
#define  __WALKABLESCRIPT_H__

#include "Script.h"
#include <MathGeoLib/MathGeoLib.h>

class WalkableScript : public Script
{
public:
	WalkableScript();
	~WalkableScript() = default;
	void Awake() override;
	void Start() override;
	void Update() override;
	void OnInspector(ImGuiContext*) override;
	void Move();

	float speed = 0.01f;
	float rotation_speed = 0.01f;
	int player_id = 0;

private:
	bool is_jumping = false;
	float current_y = 0.0f;
	float jump_power = 10.0f;
	float3 movement_vector;
	float3 gravity_vector;

};
extern "C" SCRIPT_API WalkableScript* WalkableScriptDLL(); //This is how we are going to load the script
#endif