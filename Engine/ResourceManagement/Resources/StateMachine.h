#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include "Resource.h"
#include "Animation.h"
#include "ResourceManagement/Loaders/StateMachineManager.h"
#include "EditorUI/Panel/PanelStateMachine.h"
struct Clip
{
	Clip() = default;
	Clip(std::string& name, std::shared_ptr<Animation>& animation, bool loop);
	std::string name;
	uint64_t name_hash = 0;
	std::shared_ptr<Animation> animation = nullptr;
	bool loop = false;
};

struct State
{
	State() = default;
	State(std::string name, std::shared_ptr<Clip> clip);
	std::string name;
	uint64_t name_hash = 0;
	std::shared_ptr<Clip> clip = nullptr;
};

struct Transition
{
	Transition() = default;
	Transition(uint64_t source, uint64_t target, std::string & trigger, long interpolation);
	uint64_t source_hash = 0;
	uint64_t target_hash = 0;
	uint64_t trigger_hash = 0;
	std::string trigger;
	uint64_t interpolation_time = 0;
};
class File;
class StateMachine : public Resource
{
public:
	StateMachine(std::vector<std::shared_ptr<Clip>> && clips, std::vector<std::shared_ptr<State>> && states, std::vector<std::shared_ptr<Transition>> && transitions, const std::string& file_path);
	StateMachine(const std::string& file_path);
	~StateMachine() = default;
	void LoadInMemory() override {};

	std::shared_ptr<State> GetDefaultState() const;
	std::shared_ptr<State> GetState(uint64_t state_hash) const;
	std::shared_ptr<Transition> GetTransition(const std::string & trigger, uint64_t state_hash) const;

	void Save() const;
	void Load(const File & file);

private:
	void RemoveState(const std::shared_ptr<State> & state);
	void RemoveClip(const std::shared_ptr<Clip> & state);
	void AddClipToState(std::shared_ptr<State> & state, File & clip_file);

public:
	std::vector<std::shared_ptr<Clip>> clips;
	std::vector<std::shared_ptr<State>> states;
	std::vector<std::shared_ptr<Transition>> transitions;

	friend class PanelStateMachine;
};

namespace Loader
{
	template<>
	static std::shared_ptr<StateMachine> Load(const std::string& uid) {
		return StateMachineManager::Load(uid);
	}
}
#endif // !_H_STATEMACHINE_


