#include "Importer.h"

#include "Main/Application.h"
#include "Module/ModuleFileSystem.h"
#include "Module/ModuleResourceManager.h"

#include "ResourceManagement/Metafile/MetafileManager.h"

#include <pcg_basic.h>

Metafile* Importer::Import(Path& assets_file_path)
{
	FileData imported_data = ExtractData(assets_file_path);

	Metafile* metafile;
	std::string metafile_path_string = App->resources->metafile_manager->GetMetafilePath(assets_file_path);

	if (App->filesystem->Exists(metafile_path_string))
	{
		metafile = App->resources->metafile_manager->GetMetafile(*App->filesystem->GetPath(metafile_path_string));
	}
	else
	{
		metafile = App->resources->metafile_manager->CreateMetafile(assets_file_path, resource_type);
	}

	std::string metafile_exported_folder = MetafileManager::GetMetafileExportedFolder(*metafile);
	if (!App->filesystem->Exists(metafile_exported_folder))
	{
		App->filesystem->MakeDirectory(metafile_exported_folder);
	}

	Path* metafile_exported_folder_path = App->filesystem->GetPath(metafile_exported_folder);
	metafile_exported_folder_path->Save(std::to_string(metafile->uuid).c_str(), imported_data);

	MetafileManager::TouchMetafileTimestamp(*metafile);

	return metafile;
}

bool Importer::ImportRequired(const Path& file_path)
{
	std::string metafile_path_string = App->resources->metafile_manager->GetMetafilePath(file_path);
	if (App->filesystem->Exists(metafile_path_string))
	{
		Path* metafile_path = App->filesystem->GetPath(metafile_path_string);

		Metafile* metafile = App->resources->metafile_manager->GetMetafile(*metafile_path);
		assert(App->resources->metafile_manager->IsMetafileConsistent(*metafile));

		return metafile->timestamp < file_path.modification_timestamp;
	}

	return true;
}