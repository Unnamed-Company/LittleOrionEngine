#ifndef _EDITORACTIONSETTEXTURE_H_
#define _EDITORACTIONSETTEXTURE_H_

#include "EditorAction.h"
#include "Component/ComponentMaterialRenderer.h"

class EditorActionSetTexture : public EditorAction
{
public:
	EditorActionSetTexture(ComponentMaterialRenderer* component, Texture::TextureType type);
	~EditorActionSetTexture() = default;


	void Undo();
	void Redo();

private:
	uint64_t component_UUID;
	Texture::TextureType type;
	std::shared_ptr<Texture> texture;
};

#endif _EDITORACTIONSETTEXTURE_H_