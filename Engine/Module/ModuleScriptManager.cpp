#include "ModuleScriptManager.h"
#include "Main/Globals.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleFileSystem.h"
#include "Component/ComponentScript.h"
#include "Script/Script.h"
#include "Filesystem/File.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>


bool ModuleScriptManager::Init()
{
	APP_LOG_SECTION("************ Module Manager Script ************");

	GetCurrentPath();
	patchDLL(DLL_PATH, working_directory.c_str());
	dll_file = std::make_unique<File>("Resources/Scripts/GamePlaySystem.dll");
	init_timestamp = dll_file->modification_timestamp;
	gameplay_dll = LoadLibrary("GamePlaySyste_.dll");
	LoadScriptList();

	return true;
}

update_status ModuleScriptManager::Update()
{
	
	if (!scripts.empty()) 
	{
		for (auto &component_script : scripts) 
		{
			component_script->Update();
		}
	}

	PHYSFS_Stat file_info;
	PHYSFS_stat(dll_file->file_path.c_str(), &file_info);
	last_timestamp = file_info.modtime;
	if (last_timestamp != init_timestamp) 
	{
		ReloadDLL();
		init_timestamp = last_timestamp;
	}
	
	return update_status::UPDATE_CONTINUE;
}

bool ModuleScriptManager::CleanUp()
{
	FreeLibrary(gameplay_dll);
	return true;
}

void ModuleScriptManager::InitResourceScript() 
{
	if (gameplay_dll != nullptr)
	{
		for (auto &component_script : scripts)
		{
			CREATE_SCRIPT script_func = (CREATE_SCRIPT)GetProcAddress(gameplay_dll, (component_script->name + "DLL").c_str());
			if (script_func != nullptr)
			{
				component_script->script = script_func();
				component_script->script->AddReferences(component_script->owner, App);
			}
		}
	}
}

Script* ModuleScriptManager::CreateResourceScript( const std::string& script_name, GameObject* owner) 
{
	if (gameplay_dll != nullptr)
	{
		CREATE_SCRIPT script_func = (CREATE_SCRIPT)GetProcAddress(gameplay_dll, (script_name+"DLL").c_str());
		if (script_func != nullptr)
		{
			Script* script = script_func();
			script->AddReferences(owner, App);
			return script;
		}
		return nullptr;
	}
	return nullptr;
}

ComponentScript* ModuleScriptManager::CreateComponentScript()
{
	ComponentScript* new_script = new ComponentScript();
	scripts.push_back(new_script);
	return new_script;
}

void ModuleScriptManager::RemoveComponentScript(ComponentScript * script_to_remove)
{
	auto it = std::find(scripts.begin(), scripts.end(), script_to_remove);
	if (it != scripts.end())
	{
		delete *it;
		scripts.erase(it);
	}
}

void ModuleScriptManager::LoadScriptList() 
{
	if(scripts_list.size()>0)
		scripts_list.clear();

	size_t readed_bytes;
	char* scripts_file_data = App->filesystem->Load(SCRIPT_LIST_PATH, readed_bytes);
	if (scripts_file_data != nullptr)
	{
		std::string serialized_scripts_string = scripts_file_data;
		free(scripts_file_data);

		Config scripts_config(serialized_scripts_string);

		std::vector<Config> scripts_list_configs;
		scripts_config.GetChildrenConfig("Scripts", scripts_list_configs);
		for (unsigned int i = 0; i < scripts_list_configs.size(); ++i)
		{
			scripts_list.push_back(scripts_list_configs[i].config_document.GetString());
		}
	}

}

void ModuleScriptManager::ReloadDLL() 
{
	if (gameplay_dll != nullptr) 
	{
		if (!FreeLibrary(gameplay_dll)) 
		{
			return;
		}
		else 
		{
			for (auto &component_script : scripts)
			{
				component_script->script = nullptr;
			}
			remove("GamePlaySyste_.dll");
		}
	}
	patchDLL(DLL_PATH, working_directory.c_str());
	gameplay_dll = LoadLibrary("GamePlaySyste_.dll");
	InitResourceScript();

}

void ModuleScriptManager::GetCurrentPath() 
{
	TCHAR NPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, NPath);
	working_directory = NPath;
	working_directory += "/GamePlaySyste_.dll";
	APP_LOG_ERROR("something here");
}

size_t ModuleScriptManager::CStrlastIndexOfChar(const char* str, char find_char)
{
	intptr_t i = strlen(str) - 1;
	while (i >= 0)
	{
		if (str[i] == find_char) 
		{
			return i;
		}	
		--i;
	}
	return (size_t)-1;
}

bool ModuleScriptManager::patchFileName(char* filename)
{
	size_t	dot_idx = CStrlastIndexOfChar(filename, '.');
	if (dot_idx != (size_t)-1)
	{
		filename[dot_idx - 1] = '_';
		return true;
	}
	else 
	{
		return false;
	}
		
}

bool ModuleScriptManager::CopyPDB(const char* source_file, const char* destination_file, bool overwrite_existing)
{
	return CopyFile(source_file, destination_file, !overwrite_existing);
}

bool ModuleScriptManager::patchDLL(const char* dll_path, const char patched_dll_path[MAX_PATH])
{
	// init
	char patched_pdb_path[MAX_PATH] = { '\0' };

	// open DLL and copy content to fileContent for easy parsing of the DLL content
	DWORD byte_read;
	HANDLE file = CreateFileA(dll_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;
	size_t	file_size = GetFileSize((HANDLE)file, NULL);
	BYTE*	file_content = (BYTE*)malloc(file_size);
	bool	is_file_read_ok = ReadFile((HANDLE)file, file_content, (DWORD)file_size, &byte_read, NULL);
	CloseHandle(file);
	if (!is_file_read_ok || byte_read != file_size) 
	{
		APP_LOG_ERROR("Failed to read file.\n");

	}

	// check signature
	IMAGE_DOS_HEADER dos_header = *(IMAGE_DOS_HEADER*)file_content;
	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE)
	{
		APP_LOG_ERROR("Not IMAGE_DOS_SIGNATURE\n");
	}
	// IMAGE_NT_HEADERS
	IMAGE_NT_HEADERS nt_header = *((IMAGE_NT_HEADERS*)(file_content + dos_header.e_lfanew));
	if (nt_header.Signature != IMAGE_NT_SIGNATURE) 
	{
		APP_LOG_ERROR("Not IMAGE_NT_SIGNATURE\n");
	}
	
	IMAGE_DATA_DIRECTORY debug_dir;
	if (nt_header.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC && nt_header.FileHeader.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER)) 
	{
		debug_dir = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	}
	else 
	{
		APP_LOG_ERROR("Not IMAGE_NT_OPTIONAL_HDR_MAGIC\n");
	}
		

	if (debug_dir.VirtualAddress == 0 || debug_dir.Size == 0) 
	{
		APP_LOG_ERROR("No IMAGE_DIRECTORY_ENTRY_DEBUG data\n");
	}
		
	// find debug section
	int	debug_dir_section_index = -1;
	IMAGE_SECTION_HEADER*	all_section_headers = (IMAGE_SECTION_HEADER*)(file_content + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS));
	for (int j = 0; j < nt_header.FileHeader.NumberOfSections; ++j)
	{
		IMAGE_SECTION_HEADER section_header = all_section_headers[j];
		if ((debug_dir.VirtualAddress >= section_header.VirtualAddress) && (debug_dir.VirtualAddress < section_header.VirtualAddress + section_header.Misc.VirtualSize))
		{
			debug_dir_section_index = j;
			break;
		}
	}

	// read debug section
	char*	pdb_path = nullptr;
	char	original_pdb_path[MAX_PATH];
	if (debug_dir_section_index != -1)
	{
		// loop all debug directory
		int	num_debug_dir = debug_dir.Size / (int)sizeof(IMAGE_DEBUG_DIRECTORY);
		IMAGE_SECTION_HEADER	section_header = all_section_headers[debug_dir_section_index];
		IMAGE_DEBUG_DIRECTORY*	all_image_debug_dir = (IMAGE_DEBUG_DIRECTORY*)(file_content + debug_dir.VirtualAddress - (section_header.VirtualAddress - section_header.PointerToRawData));
		for (int i = 0; i < num_debug_dir; ++i)
		{
			IMAGE_DEBUG_DIRECTORY image_debug_dir = all_image_debug_dir[i];
			if (image_debug_dir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
			{
				DWORD signature = *((DWORD*)(file_content + image_debug_dir.PointerToRawData));
				if (signature == 'SDSR')	// RSDS type, i.e. PDB70
				{
					CV_INFO_PDB70* debug_info = ((CV_INFO_PDB70*)(file_content + image_debug_dir.PointerToRawData));
					pdb_path = (char*)debug_info->PdbFileName;
					strcpy(original_pdb_path, pdb_path);
					break;
				}
			}
		}
	}

	if (pdb_path == nullptr) 
	{
		APP_LOG_ERROR("No debug section is found.\n");
	}
		
	// create new DLL and pdb
	patchFileName(pdb_path);
	if (App->filesystem->Exists(original_pdb_path))
	{
		strcpy(patched_pdb_path, pdb_path);
		CopyPDB(original_pdb_path, pdb_path, true);		// copy new PDB
	}
	HANDLE patched_dll = CreateFile(patched_dll_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD byte_write;
	WriteFile(patched_dll, file_content, (DWORD)file_size, &byte_write, nullptr);	// generate patched DLL which points to the new PDB
	CloseHandle(patched_dll);

	// clean up
	APP_LOG_ERROR("Patching DLL succeeded!!!.\n");
}