#ifndef  __PLAYERATTACK_H__
#define  __PLAYERATTACK_H__
#include "Script.h"
#include "EnemyManager.h"

class ComponentAnimation;
class ComponentCollider;

const float PUNCH_DAMAGE = 33.0f;
const float KICK_DAMAGE = 45.0f;

class PlayerAttack : public Script
{
public:
	PlayerAttack();
	~PlayerAttack() = default;

	void Awake() override;
	void Start() override;
	bool Attack();
	void ComputeCollisions() const;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;
private:	
	EnemyManager* enemy_manager = nullptr;
	ComponentAnimation* animation = nullptr;
	GameObject* collider = nullptr;
	ComponentCollider* collider_component = nullptr;
	unsigned current_damage_power = 0;
	bool is_attacking = false;
	bool raycast_cast = false;
};
extern "C" SCRIPT_API PlayerAttack* PlayerAttackDLL(); //This is how we are going to load the script
#endif