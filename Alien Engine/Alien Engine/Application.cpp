#include "Application.h"
#include "Parson/parson.h"
#include "Time.h"

Application::Application()
{
	window = new ModuleWindow();
	input = new ModuleInput();
	renderer3D = new ModuleRenderer3D();
#ifndef GAME_VERSION
	camera = new ModuleCamera3D();
	ui = new ModuleUI();
#endif
	importer = new ModuleImporter();
	objects = new ModuleObjects();
	file_system = new ModuleFileSystem();
	resources = new ModuleResources();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	LoadDll();

	// Main Modules
	AddModule(window);
#ifndef GAME_VERSION
	AddModule(camera);
#endif
	AddModule(input);
	AddModule(file_system);
	AddModule(resources);
	AddModule(importer);
	// Scenes
	AddModule(objects);
#ifndef GAME_VERSION
	AddModule(ui);
#endif
	// Renderer last!
	AddModule(renderer3D);

	window->segment_width = (WINDOW_ICON_WIDTH - BAR_BEGIN_POS * 2) / (list_modules.size() * 2);
}

void Application::LoadDll()
{
	static char curr_dir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, curr_dir);
	dll = std::string(curr_dir + std::string("/") + DLL_WORKING_PATH);
	file_system->NormalizePath(dll);

#ifndef GAME_VERSION
	if (file_system->Exists(DLL_CREATION_PATH)) {
		remove(DLL_WORKING_PATH);
		while (MoveFileA(DLL_CREATION_PATH, DLL_WORKING_PATH) == FALSE) {}
	}
#endif

	scripts_dll = LoadLibrary(dll.data());
}

Application::~Application()
{
	std::list<Module*>::reverse_iterator item = list_modules.rbegin();

	while(item != list_modules.rend())
	{
		delete(*item);
		++item;
	}

	list_modules.clear();

	std::list<JSONfilepack*>::iterator json_item = json_files.begin();
	for (; json_item != json_files.end(); ++json_item) {
		if (*json_item != nullptr) {
			delete* json_item;
			*json_item = nullptr;
		}
	}
	json_files.clear();

	if (shortcut_manager != nullptr) {
		delete shortcut_manager;
	}

	FreeLibrary(scripts_dll);
}

void Application::LoadDefaultConfig()
{
	if (FileExists("Configuration/CustomConfiguration.json")) {
		remove("Configuration/CustomConfiguration.json");
		json_files.remove(config);
		delete config;
		config = nullptr;
	}
	config = LoadJSONFile("Configuration/DefaultConfiguration.json");
	if (config != nullptr) {
		LoadConfig();
	}
}

void Application::SaveCustomConfig()
{
	remove("Configuration/CustomConfiguration.json");
	config = CreateJSONFile("Configuration/CustomConfiguration.json");
	if (config != nullptr)
		SaveConfig();
}

JSONfilepack* Application::GetJSONLayout()
{
	return layout;
}

bool Application::LoadConfig()
{
	bool ret = true;

	fps_cap = config->GetBoolean("Configuration.Application.CapFPS");
	fps_limit = config->GetNumber("Configuration.Application.LimitFPS");

	if (fps_limit > 0)
	{
		framerate_cap = 1000 / fps_limit;
	}

	std::list<Module*>::iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		(*item)->LoadConfig(config);
		++item;
	}

	return true;
}

bool Application::SaveConfig()
{
	bool ret = true;

	config->StartSave();

	config->SetBoolean("Configuration.Application.CapFPS", fps_cap);
	config->SetNumber("Configuration.Application.LimitFPS", fps_limit);

	std::list<Module*>::iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		(*item)->SaveConfig(config);
		++item;
	}

	config->FinishSave();

	return true;
}

inline bool Application::FileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool Application::Init()
{
	bool ret = true;

	if (FileExists("Configuration/CustomConfiguration.json")) { // look if it has custom settings
		config = LoadJSONFile("Configuration/CustomConfiguration.json");
	}
	else { // load default settings if no custom
		config = LoadJSONFile("Configuration/DefaultConfiguration.json");
	}

	if (config != nullptr)
		LoadConfig();

	layout = LoadJSONFile("Configuration/LayoutsInfo.json");

#ifndef GAME_VERSION
	shortcut_manager = new ShortCutManager();
#endif
	// Call Init() in all modules
	std::list<Module*>::iterator item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Init();
		window->IncreaseBar();
		++item;
	}

	// After all Init calls we call Start() in all modules
	LOG_ENGINE("Application Start --------------");
	item = list_modules.begin();

	Time::Start();
	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Start();
		window->IncreaseBar();
		++item;
	}

	ret = window->CreateCoreWindow();

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	frame_count++;
	last_sec_frame_count++;
	dt = frame_time.ReadSec();
	if (Time::IsPlaying()) {
		Time::SetDT(dt);
	}
	else {
		Time::SetDT(0);
	}
	frame_time.Start();
	ptimer.Start();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	if (last_sec_frame_time.Read() > 1000)
	{
		prev_last_sec_frame_count = last_sec_frame_count;
		last_sec_frame_count = 0;
		last_sec_frame_time.Start();
	}

	float avg_fps = float(frame_count) / startup_time.ReadSec();
	float seconds_since_startup = startup_time.ReadSec();
	unsigned __int32 last_frame_ms = frame_time.Read();
	unsigned __int32 frames_on_last_update = prev_last_sec_frame_count;

	if (framerate_cap > 0 && last_frame_ms < framerate_cap && fps_cap)
	{
		j1PerfTimer time;
		float delaytimestart = time.ReadMs();
		SDL_Delay(framerate_cap - last_frame_ms);
		float delaytimefinish = time.ReadMs();
	}
	Time::Update();
#ifndef GAME_VERSION
	ui->FramerateRegister((float)prev_last_sec_frame_count, (float)(framerate_cap));
#endif
}

JSONfilepack* Application::LoadJSONFile(const std::string& path)
{
	JSON_Value* value = json_parse_file(path.data());
	JSON_Object* object = json_value_get_object(value);

	if (value == nullptr || object == nullptr)
	{
		LOG_ENGINE("Error loading %s", path);
		return nullptr;
	}
	else {
		json_files.push_back(new JSONfilepack(path, object, value));
		return json_files.back();
	}
}

JSONfilepack* Application::CreateJSONFile(const std::string& path)
{
	JSON_Value* value = json_value_init_object();
	JSON_Object* object = json_value_get_object(value);
	json_serialize_to_file_pretty(value, path.data());
	if (value == nullptr || object == nullptr) {
		LOG_ENGINE("Error creating JSON with path %s", path.data());
		return nullptr;
	}
	else {
		json_files.push_back(new JSONfilepack(path, object, value));
		return json_files.back();
	}
}

void Application::DeleteJSONfile(JSONfilepack* json_pack)
{
	std::list<JSONfilepack*>::iterator item = json_files.begin();
	for (; item != json_files.end(); ++item) {
		if (*item != nullptr && *item == json_pack) {
			delete* item;
			*item = nullptr;
			json_files.remove(*item);
			break;
		}
	}
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	std::list<Module*>::iterator item = list_modules.begin();
	
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PreUpdate(dt);
		++item;
	}
	item = list_modules.begin();
#ifndef GAME_VERSION
	shortcut_manager->UpdateShortCuts();
#endif
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->Update(dt);
		++item;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PostUpdate(dt);
		++item;
	}
	if (quit)
		ret = UPDATE_STOP;
	FinishUpdate();

	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	std::list<Module*>::reverse_iterator item = list_modules.rbegin();
	Time::CleanUp();
	while(item != list_modules.rend() && ret == true)
	{
		ret = (*item)->CleanUp();
		++item;
	}
	return ret;
}


bool Application::StringCmp(const char* str1, const char* str2)
{
	size_t size = strlen(str1);
	if (size != strlen(str2))
		return false;

	for (uint i = 0; i < size; ++i) {
		if (std::tolower(str1[i]) != std::tolower(str2[i]))
			return false;
	}
	return true;
}

void Application::QuitApp()
{
	quit = true;
}

bool Application::IsQuiting() const
{
	return quit;
}

void Application::OpenWebsite(const std::string& website)
{
	ShellExecuteA(NULL, "open", website.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}