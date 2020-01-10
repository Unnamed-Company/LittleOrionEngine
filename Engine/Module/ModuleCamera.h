#ifndef _MODULECAMERA_H_
#define _MODULECAMERA_H_

#include "Module.h"
#include "Globals.h"
#include "GameObject.h"
#include "Skybox.h"

#include "Geometry/Frustum.h"
#include "MathGeoLib.h"


class ComponentCamera;
class GameObject;

class ModuleCamera : public Module
{
public:
	ModuleCamera() = default;
	~ModuleCamera() = default;
	
	bool Init() override;
	update_status Update() override;
	bool CleanUp() override;
	
	ComponentCamera* CreateComponentCamera();
	void RemoveComponentCamera(ComponentCamera* camera_to_remove);

	void SelectMainCamera();

	void SetOrbit(bool is_orbiting);
	bool IsOrbiting() const;

	void SetMovement(bool movement_enabled);
	bool IsMovementEnabled() const;

	void ShowCameraOptions();
	
public:
	ComponentCamera *scene_camera = nullptr;
	ComponentCamera* main_camera = nullptr;

	Skybox *skybox = nullptr;

private:
	GameObject *scene_camera_game_object = nullptr;

	bool movement_enabled = false;
	bool game_window_is_hovered = false;

	bool is_orbiting = false;
	float speed_up;


	std::vector<ComponentCamera*> cameras;

};

#endif //_MODULECAMERA_H_