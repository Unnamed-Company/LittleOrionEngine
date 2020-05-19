#ifndef  __WORLDMANAGER_H__
#define  __WORLDMANAGER_H__

#include "Script.h"
#include "EventManager.h"

class ComponentCollider;
class PlayerController;
class ComponentImage;


class WorldManager : public Script
{
public:
	WorldManager();
	~WorldManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();

	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

	bool LoadLevel() const;

private:
	void InitTriggers();
	void CheckTriggers();
	void CheckHole();

public:
	static bool singleplayer;

	//Which player is each one
	static bool player1_choice;
	static bool player2_choice;

public:
	static bool singleplayer;

	//Which player is each one
	static bool player1_choice;
	static bool player2_choice;

private:
	GameObject* health_bar = nullptr;
	GameObject* lose_screen = nullptr;
	GameObject* win_screen = nullptr;
	GameObject* player1_go = nullptr;
	GameObject* player2_go = nullptr;

	ComponentImage* lose_component = nullptr;
	ComponentImage* win_component = nullptr;

	PlayerController* player1_controller = nullptr;
	PlayerController* player2_controller = nullptr;

	EventManager* event_manager = nullptr;

	ComponentCollider* event_triggers[3];
	unsigned current_event_trigger = 0;
	bool transition = false;
	bool on_main_menu = false;

	ComponentCollider* hole = nullptr;
	bool disable_hole = false;
	float3 fall = float3(3.f, -10000.f, 0.f);
	




};
extern "C" SCRIPT_API WorldManager* WorldManagerDLL(); //This is how we are going to load the script
#endif