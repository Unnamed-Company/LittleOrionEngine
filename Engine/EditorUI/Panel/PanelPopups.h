#ifndef _PANELPOPUPS_H_
#define _PANELPOPUPS_H_

#include "Panel.h"
#include "PopupsPanel/PanelPopupMaterialSelector.h"
#include "PopupsPanel/PanelPopupMeshSelector.h"
#include "PopupsPanel/PanelPopupSceneManagement.h"

#include <string>

class PanelPopups : public Panel
{
public:
	PanelPopups();
	~PanelPopups() = default;

	void Render() override;

	void CreateScript();

private:
	void RenderAssetsLoadingPopup();

public:
	PanelPopupMaterialSelector material_selector_popup;
	PanelPopupMeshSelector mesh_selector_popup;
	PanelPopupSceneManagement scene_management_popup;

	bool create_script_shown = false;
	bool show_assets_loading_popup = false;
};

#endif //_PANELPOPUPS_H_