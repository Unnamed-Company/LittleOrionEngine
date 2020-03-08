#ifndef _COMPONENTTRANSFORM_H_
#define _COMPONENTTRANSFORM_H_

#define ENGINE_EXPORTS
#include "Component.h"
#include "UI/Panel/InspectorSubpanel/PanelComponent.h"

#include "MathGeoLib.h"
#include <GL/glew.h>

class ComponentTransform : public Component
{
public:
	ComponentTransform(GameObject * owner);
	ComponentTransform(GameObject * owner,const float3 translation, const Quat rotation, const float3 scale);

	~ComponentTransform() = default;

	void Delete() override {};

	void Save(Config& config) const override;
	void Load(const Config& config) override;
	
	ENGINE_API float3 GetGlobalTranslation() const;
	ENGINE_API float3 GetTranslation() const;
	ENGINE_API void SetTranslation(const float3& translation);
	ENGINE_API void Translate(const float3& translation);

	Quat GetRotation() const;
	ENGINE_API float3 GetRotationRadiants() const;
	ENGINE_API void SetRotation(const float3x3& rotation);
	ENGINE_API void SetRotation(const float3& rotation);
	void Rotate(const Quat& rotation);
	void Rotate(const float3x3& rotation);

	float3 GetScale() const;
	void SetScale(const float3& scale);

	float3 GetUpVector() const;
	float3 GetFrontVector() const;
	float3 GetRightVector() const;

	void ChangeLocalSpace(const float4x4& new_local_space);

	void GenerateGlobalModelMatrix();
	float4x4 GetGlobalModelMatrix() const;
	void SetGlobalModelMatrix(const float4x4& new_global_matrix);
  
private:
	void OnTransformChange();

private:
	float3 translation = float3::zero;
	Quat rotation = Quat::identity;
	float3 rotation_degrees = float3::zero;
	float3 rotation_radians = float3::zero;
	float3 scale = float3::one;

	float4x4 model_matrix = float4x4::identity;
	float4x4 global_model_matrix = float4x4::identity;

	friend class PanelComponent;
};

#endif //_COMPONENTTRANSFORM_H_