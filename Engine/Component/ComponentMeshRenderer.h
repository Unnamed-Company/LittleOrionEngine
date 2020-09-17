#ifndef _COMPONENTMESHRENDERER_H_
#define _COMPONENTMESHRENDERER_H_

#include "Component.h"
#include "ResourceManagement/Resources/Mesh.h"
#include "ResourceManagement/Resources/Material.h"
#include "ResourceManagement/Resources/Skeleton.h"
#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

class ComponentMeshRenderer : public Component
{
public:
	enum Variations
	{
		ENABLE_NORMAL_MAP =1 << 0,
		ENABLE_SPECULAR_MAP = 1 << 1
	};
	ComponentMeshRenderer();
	ComponentMeshRenderer(GameObject * owner);
	~ComponentMeshRenderer() = default;

	ComponentMeshRenderer(const ComponentMeshRenderer& component_to_copy) = default;
	ComponentMeshRenderer(ComponentMeshRenderer&& component_to_move) = default;

	ComponentMeshRenderer& operator=(const ComponentMeshRenderer& component_to_copy) = default;
	ComponentMeshRenderer& operator=(ComponentMeshRenderer&& component_to_move) = default;

	Component* Clone(GameObject* owner, bool original_prefab) override;
	void CopyTo(Component* component_to_copy) const override;

	void SpecializedSave(Config& config) const override;
	void SpecializedLoad(const Config& config) override;

	void LoadResource(uint32_t uuid, ResourceType resource, unsigned texture_type) override;
	void InitResource(uint32_t uuid, ResourceType resource, unsigned texture_type) override;
	void ReassignResource() override;

	void Delete() override;

	GLuint BindShaderProgram() const;
	GLuint BindDepthShaderProgram() const;
	void BindMeshUniforms(GLuint shader_program) const;
	void BindMaterialUniforms(GLuint shader_program) const;
	void RenderModel() const;

	void SetMesh(uint32_t mesh_uuid);
	void SetMaterial(uint32_t material_uuid);
	void SetSkeleton(uint32_t skeleton_uuid);

	void UpdatePalette(std::vector<float4x4> & pose);

private:
	void AddDiffuseUniforms(unsigned int shader_program) const;
	void AddEmissiveUniforms(unsigned int shader_program) const;
	void AddSpecularUniforms(unsigned int shader_program) const;
	void AddAmbientOclusionUniforms(unsigned int shader_program) const;
	void AddNormalUniforms(unsigned int shader_program) const;
	void AddLightMapUniforms(unsigned int shader_program) const;
	void AddLiquidMaterialUniforms(unsigned int shader_program) const;
	void AddDissolveMaterialUniforms(unsigned int shader_program) const;
	void AddExtraUniforms(unsigned int shader_program) const;

	bool BindTexture(Material::MaterialTextureType id) const;
	bool BindTextureNormal(Material::MaterialTextureType id) const;

public:
	uint32_t mesh_uuid;
	std::shared_ptr<Mesh> mesh_to_render = nullptr;

	uint32_t material_uuid;
	std::shared_ptr<Material> material_to_render = nullptr;

	uint32_t skeleton_uuid;
	std::shared_ptr<Skeleton> skeleton = nullptr;

	std::vector<float4x4> palette;

	bool is_raycastable = true;
	ComponentMeshCollider* mesh_collider = nullptr;
	bool shadow_caster = false;
	bool shadow_receiver = false;
private:
	friend class PanelComponent;
};

#endif //_COMPONENTMESHRENDERER_H_