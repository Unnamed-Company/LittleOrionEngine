#ifndef  __CAMERACONTROLLER_H__
#define  __CAMERACONTROLLER_H__

#include "Script.h"

#include "MathGeoLib.h"

class TestScriptRuntime;
class ComponentCamera;

class CameraController : public Script
{
public:
	CameraController();
	~CameraController() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void GodCamera();
	void ActivePlayer();
	void FollowPlayer();
	void CenterToPlayer();

	//void Save(Config& config) const override;
	//void Load(const Config& config) override;
	//void Link() override;

	void InitPublicGameObjects();

private:
	bool god_mode = false;

	GameObject* camera = nullptr;
	ComponentCamera* camera_component = nullptr;

	GameObject* player = nullptr;
	TestScriptRuntime* player_movement_script = nullptr;
	ComponentScript* player_movement_component = nullptr;

};
extern "C" SCRIPT_API CameraController* CameraControllerDLL(); //This is how we are going to load the script
#endif