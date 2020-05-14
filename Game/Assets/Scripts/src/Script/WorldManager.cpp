#include "WorldManager.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Filesystem/PathAtlas.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include "imgui.h"



WorldManager* WorldManagerDLL()
{
	WorldManager* instance = new WorldManager();
	return instance;
}

WorldManager::WorldManager()
{
	panel = new PanelComponent();
}

// Use this for initialization before Start()
void WorldManager::Awake()
{
	//health_component = (ComponentProgressBar*)health_bar->GetComponent(Component::ComponentType::UI_PROGRESS_BAR);
	player_controller = (ComponentScript*)player->GetComponentScript("PlayerController");

	GameObject* event_manager_go = App->scene->GetGameObjectByName("EventManager");
	ComponentScript* event_manager_component = event_manager_go->GetComponentScript("EventManager");
	event_manager = static_cast<EventManager*>(event_manager_component->script);

	InitTriggers();
}

// Use this for initialization
void WorldManager::Start()
{

}

// Update is called once per frame
void WorldManager::Update()
{
	CheckTriggers();
	/*
	if(health_component->percentage <= 0.0f)
	{
		//Lose
		player_controller->Disable();
		lose_component->Enable();
		transition = true;
	}

	if(OnTriggerEnter())
	{
		//Win
		player_controller->Disable();
		win_component->Enable();
		transition = true;
	}

	if(transition && App->input->GetAnyKeyPressedDown())
	{
		App->scene->LoadScene(0);
	}
	*/

}

// Use this for showing variables on inspector
void WorldManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

//Use this for linking JUST GO automatically
void WorldManager::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	//public_gameobjects.push_back(&health_bar);
	//variable_names.push_back(GET_VARIABLE_NAME(health_bar));

	//public_gameobjects.push_back(&lose_screen);
	//variable_names.push_back(GET_VARIABLE_NAME(lose_screen));

	//public_gameobjects.push_back(&win_screen);
	//variable_names.push_back(GET_VARIABLE_NAME(win_screen));

	public_gameobjects.push_back(&player);
	variable_names.push_back(GET_VARIABLE_NAME(player));



	for (unsigned i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}
void WorldManager::InitTriggers()
{
	GameObject* trigger_go_dad = App->scene->GetGameObjectByName("Triggers");
	for(size_t i = 0; i < trigger_go_dad->children.size();++i)
	{
		event_triggers[i] = static_cast<ComponentCollider*>(trigger_go_dad->children[i]->GetComponent(ComponentCollider::ColliderType::BOX));
	}
}
void WorldManager::CheckTriggers()
{
	if(current_event_trigger > 2)
	{
		return;
	}

	if(static_cast<ComponentCollider*>(player->GetComponent(ComponentCollider::ColliderType::CAPSULE))->DetectCollisionWith(event_triggers[current_event_trigger]))
	{
		if(event_manager->TriggerEvent(current_event_trigger))
		{
			++current_event_trigger;
		}
	}

}

//Use this for linking GO AND VARIABLES automatically if you need to save variables
// void WorldManager::Save(Config& config) const
// {
// 	config.AddUInt(example->UUID, "ExampleNameforSave");
// 	Script::Save(config);
// }

// //Use this for linking GO AND VARIABLES automatically
// void WorldManager::Load(const Config& config)
// {
// 	exampleUUID = config.GetUInt("ExampleNameforSave", 0);
// 	Script::Load(config);
// }
