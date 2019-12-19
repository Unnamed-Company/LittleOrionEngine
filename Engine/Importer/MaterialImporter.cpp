#include "Texture.h"
#include "MaterialImporter.h"

#include "Application.h"
#include <algorithm>

MaterialImporter::MaterialImporter()
{

	APP_LOG_INIT("Initializing DevIL image loader.")
	ilInit();
	iluInit();
	ilutInit();
	APP_LOG_SUCCESS("DevIL image loader initialized correctly.")

}
bool MaterialImporter::Import(const char* file_path, std::string& output_file)
{
	ModuleFileSystem::File file = ModuleFileSystem::File(file_path);
	if (file.filename.empty())
	{
		APP_LOG_SUCCESS("Importing material error: Couldn't find the file to import.")
		return false;
	}
	//Bound image

	ILuint image;
	ilGenImages(1, &image);
	ilBindImage(image);
	int width, height;
	ILubyte * save_data = LoadImageData(file_path, width, height, IL_RGBA);
	//Get new Name

	std::string texture_name_no_extension = file.filename.substr(0, file.filename.find_last_of("."));
	output_file = LIBRARY_TEXTURES_FOLDER + "//" + texture_name_no_extension + ".dds";

	//Save data
	ILuint size;

	ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);// To pick a specific DXT compression use
	size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
	if (size > 0) {

		if (ilSaveL(IL_DDS, save_data, size) > 0) // Save to buffer with the ilSaveIL function
		{
			App->filesystem->Save(output_file.c_str(), (unsigned int *)save_data, size);
		}
		ilDeleteImages(1, &image);
	}
	return true;
}

std::shared_ptr<Texture> MaterialImporter::Load(const char* file_path) {

	//Check if the mesh is already loaded
	auto it = std::find_if(texture_cache.begin(), texture_cache.end(), [file_path](const std::shared_ptr<Texture> texture)
	{
		return texture->texture_path == file_path;
	});
	if (it != texture_cache.end())
	{
		APP_LOG_INIT("Model %s exists in cache.", file_path);
		return *it;
	}

	ILuint image;
	ilGenImages(1, &image);
	ilBindImage(image);

	int width, height;
	ILubyte * data = LoadImageData(file_path, width, height, IL_DDS);

	if (data == NULL)
	{
		ilDeleteImages(1, &image);
		return nullptr;
	}
	std::shared_ptr<Texture> loaded_texture = std::make_shared<Texture>(data, width, height, file_path);
	loaded_texture->GenerateMipMap();
	ilDeleteImages(1, &image);
	texture_cache.push_back(loaded_texture);
	return loaded_texture;
}

ILubyte * MaterialImporter::LoadImageData(const char* file_path, int & width, int & height, int image_type ) const
{
	ilLoadImage(file_path);

	ILenum error;
	error = ilGetError();
	if (error == IL_COULD_NOT_OPEN_FILE)
	{
		APP_LOG_ERROR("Error loading texture %s. File not found", file_path);
		return nullptr;
	}

	ilConvertImage(image_type, IL_UNSIGNED_BYTE);

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);
	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT && image_type != IL_DDS)
	{
		iluFlipImage();
	}

	ILubyte * data = ilGetData();
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	return data;
}

unsigned int MaterialImporter::LoadCubemap(std::vector<std::string> faces_paths) const
{
	unsigned int texture_id;

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	ILuint image;

	int width, height;
	for (unsigned int i = 0; i < faces_paths.size(); i++)
	{
		ilGenImages(1, &image);
		ilBindImage(image);
		unsigned char * data = LoadImageData(faces_paths[i].c_str(), width, height, IL_DDS);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			ilDeleteImages(1, &image);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texture_id;
}

//Remove the mesh from the cache if the only owner is the cache itself
void MaterialImporter::RemoveTextureFromCacheIfNeeded(std::shared_ptr<Texture> texture) 
{
	auto it = std::find(texture_cache.begin(), texture_cache.end(), texture);
	if (it != texture_cache.end() && (*it).use_count() <= 2)
	{
		(*it).~shared_ptr();
	}
}