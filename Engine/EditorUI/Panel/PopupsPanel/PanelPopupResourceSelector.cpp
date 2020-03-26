#include "PanelPopupResourceSelector.h"

#include "Component/ComponentMeshRenderer.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleResourceManager.h"
#include "Module/ModuleTexture.h"

#include <imgui.h>

PanelPopupResourceSelector::PanelPopupResourceSelector()
{
	opened = false;
	enabled = true;
	window_name = "Popups Resource Selector";
}

void PanelPopupResourceSelector::ShowPanel(ResourceType resource_type)
{
	show_resource_selector_popup = true;
	this->resource_type = resource_type;
}

void PanelPopupResourceSelector::Render()
{
	if (show_resource_selector_popup)
	{
		show_resource_selector_popup = false;
		opened = true;

		resource_metafiles = std::vector<Metafile*>();
		App->resources->resource_DB->GetEntriesOfType(resource_metafiles, resource_type);
		resource_name = GetResourceName();
	}

	if (!opened)
	{
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(resource_icon_size * 4.5, resource_icon_size * 2 * 1.1f));

	if (ImGui::Begin(("Select " + resource_name).c_str(), &opened))
	{
		hovered = ImGui::IsWindowHovered();

		child_window_focused = false;

		ImVec2 available_region = ImGui::GetContentRegionAvail();
		int files_per_line = available_region.x / resource_icon_size;

		int current_line = 0;
		int current_file_in_line = 0;

		for (auto& resource_metafile : resource_metafiles)
		{
			ImGui::PushID(current_line * files_per_line + current_file_in_line);
			ShowResourceIcon(resource_metafile);
			ImGui::PopID();

			++current_file_in_line;
			if (current_file_in_line == files_per_line)
			{
				current_file_in_line = 0;
				++current_line;
			}
			else
			{
				ImGui::SameLine();
			}

		}

		if (child_window_focused)
		{
			ImGui::SetWindowFocus();
		}
		focused = ImGui::IsWindowFocused();

		if (!focused)
		{
			opened = false;
		}
	}
	ImGui::End();
}

void PanelPopupResourceSelector::ShowResourceIcon(Metafile* resource_metafile)
{
	Path* resource_imported_file_path = App->filesystem->GetPath(resource_metafile->imported_file_path);
	std::string filename = resource_imported_file_path->file_name_no_extension;
	if (ImGui::BeginChild(filename.c_str(), ImVec2(resource_icon_size, resource_icon_size), selected_resource_metafile == resource_metafile, ImGuiWindowFlags_NoDecoration))
	{
		ProcessMouseInput(resource_metafile);

		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 0.75 * resource_icon_size) * 0.5f);
		ImGui::Image((void *)App->texture->whitefall_texture_id, ImVec2(0.75*resource_icon_size, 0.75 * resource_icon_size)); // TODO: Substitute this with resouce thumbnail
		ImGui::Spacing();

		float text_width = ImGui::CalcTextSize(filename.c_str()).x;
		if (text_width < resource_icon_size)
		{
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_width) * 0.5f);
			ImGui::Text(filename.c_str());
		}
		else
		{
			int character_width = text_width / filename.length();
			int string_position_wrap = resource_icon_size / character_width - 5;
			assert(string_position_wrap < filename.length());
			std::string wrapped_filename = filename.substr(0, string_position_wrap) + "...";
			ImGui::Text(wrapped_filename.c_str());
		}
	}
	ImGui::EndChild();
}

void PanelPopupResourceSelector::ProcessMouseInput(Metafile* file)
{
	if (ImGui::IsWindowHovered())
	{
		if (ImGui::IsMouseClicked(0))
		{
			if (selected_resource_metafile != file)
			{
				selected_resource_metafile = file;
				ChangeSelectedObjectResource();
			}
		}

		if (ImGui::IsMouseDoubleClicked(0))
		{
			opened = false;
		}
	}

	if (ImGui::IsWindowFocused())
	{
		child_window_focused = true;
	}
}

void PanelPopupResourceSelector::ChangeSelectedObjectResource() const
{
	GameObject* selected_gameobject = App->editor->selected_game_object;
	std::shared_ptr<Resource> selected_resource = App->resources->Load(selected_resource_metafile->uuid);

	//TODO: Improve this.
	Component* selected_gameobject_mesh_renderer_component = selected_gameobject->GetComponent(Component::ComponentType::MESH_RENDERER);
	assert(selected_gameobject_mesh_renderer_component != nullptr);
	ComponentMeshRenderer* selected_gameobject_mesh_renderer = static_cast<ComponentMeshRenderer*>(selected_gameobject_mesh_renderer_component);

	switch (resource_type)
	{
	case ResourceType::MATERIAL:
		selected_gameobject_mesh_renderer->SetMaterial(std::static_pointer_cast<Material>(selected_resource));
		selected_gameobject_mesh_renderer->modified_by_user = true;
		break;

	case ResourceType::MESH:
		selected_gameobject_mesh_renderer->SetMesh(std::static_pointer_cast<Mesh>(selected_resource));
		selected_gameobject_mesh_renderer->modified_by_user = true;
		break;
	}
}

std::string PanelPopupResourceSelector::GetResourceName() const
{
	switch (resource_type)
	{
	case ResourceType::ANIMATION:
		return "Animation";
		break;
	case ResourceType::AUDIO:
		return "Audio";
		break;
	case ResourceType::MATERIAL:
		return "Materila";
		break;
	case ResourceType::MESH:
		return "Mesh";
		break;
	case ResourceType::NAVMESH:
		return "NavMesh";
		break;
	case ResourceType::SKELETON:
		return "Skeleton";
		break;
	case ResourceType::SKYBOX:
		return "Skybox";
		break;
	case ResourceType::TEXTURE:
		return "Texture";
		break;
	default:
		return "Unknown";
		break;
	}
}