#include "Path.h".

#include "File.h"

#include "Main/Application.h"
#include "Module/ModuleFileSystem.h"

#include <SDL/SDL.h>

Path::Path(const std::string& path) 
{
	assert(App->filesystem->Exists(path));
	CleanFolderPath();
	file_path = path;
	file_name = path.substr(path.find_last_of('/') + 1, -1);
	file_name_no_extension = file_name.substr(0, file_name.find_last_of("."));

	Refresh();
}

Path::Path(const std::string& path, const std::string& name)
{
	assert(App->filesystem->Exists(path));
	CleanFolderPath();
	file_path = path + "/" + name;
	file_name = name;
	file_name_no_extension = file_name.substr(0, file_name.find_last_of("."));

	Refresh();
}

Path::~Path()
{
	for (auto& path_child : children)
	{
		delete path_child;
	}

	delete file;
}

bool Path::operator==(const Path& compare)
{
	return file_name == compare.file_name && file_path == compare.file_path;
}

void Path::Refresh()
{
	for (auto& path_child : children)
	{
		delete path_child;
	}
	children.clear();

	delete file;

	CalculatePathInfo();
	if (is_directory)
	{
		CalculateChildren();
	}
}

Path* Path::Save(const char* file_name, const FileData& data, bool append)
{
	assert(is_directory);
	if (data.size == 0)
	{
		return nullptr;
	}

	std::string saved_file_path_string = file_path + "/" + file_name;

	PHYSFS_File * file;
	if (append)
	{
		file = PHYSFS_openAppend(saved_file_path_string.c_str());
	}
	else
	{
		file = PHYSFS_openWrite(saved_file_path_string.c_str());
	}

	if (file == NULL)
	{
		APP_LOG_ERROR("Error: Unable to open file! PhysFS Error: %s\n", PHYSFS_getLastErrorCode());
		return nullptr;
	}


	PHYSFS_writeBytes(file, data.buffer, data.size);
	APP_LOG_INFO("File %s saved!\n", file_path);
	PHYSFS_close(file);
	
	Path* saved_file_path = new Path(saved_file_path_string);
	children.push_back(saved_file_path);
	saved_file_path->parent = this;
	App->filesystem->AddPath(saved_file_path);

	free((char*) data.buffer);

	return saved_file_path;
}

Path* Path::Save(const char* file_name, const std::string& serialized_data, bool append)
{
	char* data_bytes = new char[serialized_data.size() + 1];
	memcpy(data_bytes, serialized_data.c_str(), serialized_data.size() + 1);

	FileData data{ data_bytes, serialized_data.size() + 1 };
	return Save(file_name, data);
}

Path* Path::GetParent()
{
	return parent;
}

std::string Path::GetParentPathString(const std::string& path)
{
	std::size_t found = path.find_last_of("/");
	if (found == std::string::npos || found == 0) {
		return "";
	}
	return path.substr(0, found);
}


File* Path::GetFile() const
{
	if (file == nullptr)
	{
		APP_LOG_ERROR("Path %s doesn't contain a file!", file_path.c_str())
	}

	return file;
}

void Path::CalculatePathInfo()
{
	PHYSFS_Stat path_info;
	if (PHYSFS_stat(file_path.c_str(), &path_info) == 0)
	{
		APP_LOG_ERROR("Error getting %s path info: %s", this->file_path.c_str(), PHYSFS_getLastError());
	}

	std::string file_extension = GetExtension();
	is_directory = (PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY == path_info.filetype);
	CalculateFile();

	modification_timestamp = path_info.modtime;
	return;
}

void Path::CalculateFile()
{
	if (is_directory)
	{
		file = nullptr;
	}
	else
	{
		file = new File(*this);
	}
}

void Path::CalculateChildren()
{
	std::vector<Path*> path_children;
	GetAllFilesInPath(path_children);

	if (file_name == "Assets")
	{
		int x = 0;
	}
	for (auto & path_child : path_children)
	{
		path_child->parent = this;
		this->children.push_back(path_child);
		if (path_child->IsDirectory())
		{
			++sub_folders;
			total_sub_files_number += path_child->total_sub_files_number;
		}
		else
		{
			++total_sub_files_number;
		}
	}
}

void Path::GetAllFilesInPath(std::vector<Path*>& path_children, bool directories_only)
{
	char **files_array = PHYSFS_enumerateFiles(file_path.c_str());
	if (*files_array == NULL)
	{
		APP_LOG_INFO("Error reading directory %s: %s", file_path.c_str(), PHYSFS_getLastError());
		return;
	}

	char **filename;
	for (filename = files_array; *filename != NULL; filename++)
	{
		Path* child_path = new Path(file_path, *filename);
		if ((directories_only && child_path->is_directory) || !directories_only)
		{
			path_children.push_back(child_path);
		}
	}
	PHYSFS_freeList(files_array);
}

bool Path::IsDirectory() const
{
	return is_directory;
}

bool Path::IsMeta() const
{
	return !is_directory && file->GetFileType() == FileType::META;
}

bool Path::IsImportable() const
{
	FileType file_type = file->GetFileType();
	return
		file_type == FileType::ANIMATION
		|| file_type == FileType::MATERIAL
		|| file_type == FileType::MESH
		|| file_type == FileType::MODEL
		|| file_type == FileType::PREFAB
		|| file_type == FileType::SKELETON
		|| file_type == FileType::TEXTURE;
}

std::string Path::GetExtension() const
{
	std::size_t found = file_path.find_last_of(".");
	if (found == std::string::npos || found == 0) {
		return "";
	}
	std::string file_extension = file_path.substr(found + 1, file_path.length());

	return file_extension;
}

void Path::CleanFolderPath()
{
	if (!file_path.empty() && file_path.back() == '/')
	{
		file_path.pop_back();
	}
}