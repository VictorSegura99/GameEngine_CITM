#pragma once

#include "Module.h"
#include <vector>

struct SDL_RWops;
int close_sdl_rwops(SDL_RWops* rw);

struct aiFileIO;
#include "Bass/include/bass.h"
//struct BASS_FILEPROCS;

// -------Foldres Paths--------
#define ASSETS_FOLDER "Assets/"
#define LIBRARY_FOLDER "Library/"
#define LIBRARY_MODELS_FOLDER "Library/Models/"
#define LIBRARY_MESHES_FOLDER "Library/Meshes/"
#define LIBRARY_TEXTURES_FOLDER "Library/Textures/"
#define LIBRARY_SCENES_FOLDER "Library/Scenes/"
#define LIBRARY_PREFABS_FOLDER "Library/Prefabs/"
#define CONFIGURATION_FOLDER "Configuration/"
#define CONFIGURATION_LAYOUTS_FOLDER "Configuration/Layouts/"
#define MODELS_FOLDER "Assets/Models/"
#define TEXTURES_FOLDER "Assets/Textures/"
#define SCRIPTS_FOLDER "Assets/Scripts/"
#define SCENE_FOLDER "Assets/Scenes/"
#define ASSETS_PREFAB_FOLDER "Assets/Prefabs/"
#define LIBRARY_SCRIPTS_FOLDER "Library/Scripts/"
#define HEADER_SCRIPTS_FILE "AlienEngineScripts/"
// -------Foldres Paths--------

// -------DLL Paths--------
#define SCRIPTS_DLL_OUTPUT "AlienEngineScripts/OutPut/"
#define DLL_WORKING_PATH "DLLs/AlienEngineScripts.dll"
#define DLL_CREATION_PATH "AlienEngineScripts/OutPut/AlienEngineScripts.dll"
// -------DLL Paths--------

#define FILE_TAGS "Configuration/Tags/tags.alienTags"
#define BUILD_SETTINGS_PATH "Configuration/BuildSettings.alienBuild"
#define BUILD_EXE_PATH "../Alien Engine/EngineBuild/Alien Engine.exe"
#define BUILD_DIRECT_ACCESS_PATH "../Alien Engine/EngineBuild/DirectAccess.lnk"

// -------Templates--------
#define EXPORT_FILE_CLASS_TEMPLATE "Configuration/Script Templates/ExportClass.alienTemplate"
#define CLASS_FILE_TEMPLATE "Configuration/Script Templates/Class.alienTemplate"
#define EXPORT_FILE_STRUCT_TEMPLATE "Configuration/Script Templates/ExportStruct.alienTemplate"
#define STRUCT_FILE_TEMPLATE "Configuration/Script Templates/Struct.alienTemplate"
#define CPP_FILE_TEMPLATE "Configuration/Script Templates/BaseCPP.alienTemplate"
#define CPP_ALIEN_FILE_TEMPLATE "Configuration/Script Templates/AlienCPP.alienTemplate"
// -------Templates--------

#include "Resource_.h"

class ResourceTexture;
class FileNode;

enum class FileDropType {
	MODEL3D,
	TEXTURE,
	FOLDER,
	SCRIPT,
	SCENE,
	PREFAB,

	UNKNOWN
};


class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(const char* game_path = nullptr);

	// Destructor
	~ModuleFileSystem();

	// Called before render is available
	bool Init();
	update_status PreUpdate(float dt);
	// Called before quitting
	bool CleanUp() override;

	// Utility functions
	bool AddPath(const char* path_or_zip);
	bool Exists(const char* file) const;
	bool ExistsInFolderRecursive(const char* folder, const char* file_name);
	bool IsDirectory(const char* file) const;
	void CreateDirectory(const char* directory);
	void DiscoverFiles(const char* directory, std::vector<std::string>& file_list, std::vector<std::string>& dir_list, bool files_hole_path = false) const;
	void DiscoverEverythig(FileNode* node);
	void DiscoverFolders(FileNode* node);
	bool CopyFromOutsideFS(const char* full_path, const char* destination);
	bool Copy(const char* source, const char* destination);
	void SplitFilePath(const char* full_path, std::string* path, std::string* file = nullptr, std::string* extension = nullptr) const;
	void NormalizePath(char* full_path) const;
	void NormalizePath(std::string& full_path) const;

	bool CreateNewFile(const char* path);

	// Open for Read/Write
	unsigned int Load(const char* path, const char* file, char** buffer) const;
	unsigned int Load(const char* file, char** buffer) const;
	SDL_RWops* Load(const char* file) const;
	void* BassLoad(const char* file) const;

	// IO interfaces for other libs to handle files via PHYSfs
	aiFileIO* GetAssimpIO();
	BASS_FILEPROCS* GetBassIO();

	unsigned int Save(const char* file, const void* buffer, unsigned int size, bool append = false) const;
	bool SaveUnique(std::string& output, const void* buffer, uint size, const char* path, const char* prefix, const char* extension);
	bool Remove(const char* file);

	const char* GetBasePath() const;
	const char* GetWritePath() const;
	const char* GetReadPaths() const;

	void ManageNewDropFile(const char* extern_path);

	const FileDropType& SearchExtension(const std::string& extern_path);

	std::string GetBaseFileName(const char* file_name);
	std::string GetBaseFileNameWithExtension(const char* file_name);
	std::string GetCurrentFolder(const std::string & path);
	std::string GetCurrentHolePathFolder(const std::string& path);
	void GetPreviousNames(std::string& previous, FileNode* node);
	std::string GetPathWithoutExtension(const std::string& path);
private:

	void CreateAssimpIO();
	void CreateBassIO();

private:

	aiFileIO* AssimpIO = nullptr;
	BASS_FILEPROCS* BassIO = nullptr;

	time_t last_mod_dll = 0;
};

