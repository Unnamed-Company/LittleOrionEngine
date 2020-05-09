#ifndef  __ENEMYCONTROLLER_H__
#define  __ENEMYCONTROLLER_H__
#include "Script.h"

#include "EnemyManager.h"

class EnemyController : public Script
{
public:
	EnemyController();
	~EnemyController() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();

	EnemyManager* punterito = nullptr;

private:
	void Death();

private:
	GameObject* player = nullptr;

};
extern "C" SCRIPT_API EnemyController* EnemyControllerDLL(); //This is how we are going to load the script
#endif