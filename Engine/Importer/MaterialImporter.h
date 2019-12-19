#ifndef _MATERIALIMPORTER_H_
#define _MATERIALIMPORTER_H_

#include "Importer/Importer.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

class Texture;

class MaterialImporter : Importer
{
public:
	MaterialImporter();
	bool Import(const char* file_path, std::string& output_file) override;
	std::shared_ptr<Texture> Load(const char* file_path);
	unsigned int LoadCubemap(std::vector<std::string> faces_paths) const;

	void RemoveTextureFromCacheIfNeeded(std::shared_ptr<Texture> texture);

private:
	ILubyte* LoadImageData(const char* file_path, int & width, int & height, int image_type) const;
private:
	const std::string LIBRARY_TEXTURES_FOLDER = ".//Library//Materials";
	std::vector<std::shared_ptr<Texture>> texture_cache;
};
#endif // !_MATERIALIMPORTER_H_
