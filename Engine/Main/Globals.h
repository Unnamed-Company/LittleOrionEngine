#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include "UI/EngineLog.h"

#include "imgui.h"

#include <windows.h>
#include <stdio.h>

#define APP_LOG_SECTION(format, ...) logentry(EngineLog::LogEntrySource::APP_LOG, EngineLog::LogEntryType::LOG_SECTION, __FILE__, __LINE__, format, __VA_ARGS__);
#define APP_LOG_INFO(format, ...) logentry(EngineLog::LogEntrySource::APP_LOG, EngineLog::LogEntryType::LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__);
#define APP_LOG_INIT(format, ...) logentry(EngineLog::LogEntrySource::APP_LOG, EngineLog::LogEntryType::LOG_INIT, __FILE__, __LINE__, format, __VA_ARGS__);
#define APP_LOG_SUCCESS(format, ...) logentry(EngineLog::LogEntrySource::APP_LOG, EngineLog::LogEntryType::LOG_SUCCESS, __FILE__, __LINE__, format, __VA_ARGS__);
#define APP_LOG_ERROR(format, ...) logentry(EngineLog::LogEntrySource::APP_LOG, EngineLog::LogEntryType::LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__);

#define OPENGL_LOG_INFO(format, ...) logentry(EngineLog::LogEntrySource::OPENGL_LOG, EngineLog::LogEntryType::LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__);
#define OPENGL_LOG_INIT(format, ...) logentry(EngineLog::LogEntrySource::OPENGL_LOG, EngineLog::LogEntryType::LOG_INIT, __FILE__, __LINE__, format, __VA_ARGS__);
#define OPENGL_LOG_SUCCESS(format, ...) logentry(EngineLog::LogEntrySource::OPENGL_LOG, EngineLog::LogEntryType::LOG_SUCCESS, __FILE__, __LINE__, format, __VA_ARGS__);
#define OPENGL_LOG_ERROR(format, ...) logentry(EngineLog::LogEntrySource::OPENGL_LOG, EngineLog::LogEntryType::LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__);

#define MYASSIMP_LOG_INFO(format, ...) logentry(EngineLog::LogEntrySource::ASSIMP_LOG, EngineLog::LogEntryType::LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__);
#define MYASSIMP_LOG_INIT(format, ...) logentry(EngineLog::LogEntrySource::ASSIMP_LOG, EngineLog::LogEntryType::LOG_INIT, __FILE__, __LINE__, format, __VA_ARGS__);
#define MYASSIMP_LOG_SUCCESS(format, ...) logentry(EngineLog::LogEntrySource::ASSIMP_LOG, EngineLog::LogEntryType::LOG_SUCCESS, __FILE__, __LINE__, format, __VA_ARGS__);
#define MYASSIMP_LOG_ERROR(format, ...) logentry(EngineLog::LogEntrySource::ASSIMP_LOG, EngineLog::LogEntryType::LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__);

void logentry(const EngineLog::LogEntrySource source, const EngineLog::LogEntryType type, const char file[], const int line, const char* format, ...);


static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

enum class update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

enum class Fonts
{
	FONT_FA = 0,
	FONT_FAR = 1,
	FONT_FAB = 2
};

// Paths -------
# define APP_ICON_PATH "Resources/Textures/engine_icon.png"

# define TEXTURES_PATH "Assets/Textures/"
# define SHADERS_PATH "Resources/Shaders/shaders.json"

# define HOUSE_MODEL_PATH "Assets/Models/baker_house/BakerHouse.fbx"
# define DEFAULT_MATERIAL_PATH "Resources/Materials/default.olmat"

# define VIDEO_BILLBOARD_TEXTURE_PATH "Resources/Textures/video-solid.png"
# define LIGHT_BILLBOARD_TEXTURE_PATH "Resources/Textures/lightbulb-solid.png"

# define PRIMITIVE_CUBE_PATH "Resources/Primitives/Cube/Cube.ol"
# define PRIMITIVE_CYLINDER_PATH "Resources/Primitives/Cylinder/Cylinder.ol"
# define PRIMITIVE_SPHERE_PATH "Resources/Primitives/Sphere/Sphere.ol"
# define PRIMITIVE_TORUS_PATH "Resources/Primitives/Torus/Torus.ol"
# define PRIMITIVE_QUAD_PATH "Resources/Primitives/Quad/Quad.ol"

# define DEFAULT_SCENE_PATH "Resources/Scenes/default_scene.scene"
# define ASSIGNMENT_SCENE_PATH "Resources/Scenes/assignment_scene.scene"
# define SAVED_SCENE_PATH "Assets/Scenes/scene.scene"
# define TMP_SCENE_PATH "Assets/Scenes/tmp_scene.scene"
# define GAME_INPUT_PATH "Resources/GameInputs/game_inputs.inputs"

# define SCRIPT_LIST_PATH "Resources/Scripts/scripts_list.scripts"
# define SCRIPTS_DLL_PATH "Resources/Scripts/GamePlaySystem.dll"
# define SCRIPT_DLL_FILE "GamePlaySyste_.dll"

# define SCRIPT_TEMPLATE_FILE_CPP "Resources/Scripts/TemplateScript.cpp"
# define SCRIPT_TEMPLATE_FILE_H "Resources/Scripts/TemplateScript.h"
# define SCRIPT_PATH "../GamePlaySystem/src/Script/"

 

# define OPENGL_MAJOR_VERSION 4
# define OPENGL_MINOR_VERSION 0

#define FULLSCREEN false
#define RESIZABLE true
#define BORDERED true

#define VSYNC true
#define TITLE "Little Orion Engine"


#endif //_GLOBALS_H_