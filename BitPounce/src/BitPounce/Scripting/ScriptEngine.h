#pragma once
#include <angelscript.h>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <string>
#include <entt/entt.hpp>

struct ScriptEngineData;

namespace BitPounce
{

	class Scene;
	class Entity;              
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static void Build();
		static void BuildScriptsDirs(std::vector<std::filesystem::path> dirs);
		static void BuildScriptsFiles(std::vector<std::filesystem::path> files);
		static void BuildScripts(const std::vector<std::filesystem::path>& dirs);
		static asIScriptFunction* GetFunction(const std::string& moduleName, const std::string& funcDecl);
		static asIScriptEngine* GetEngine();

		static Entity* GetOrCreateSharedEntity(entt::entity handle, Scene* scene);
		static void InvalidateSharedEntity(entt::entity handle);

	private:
		static void InitAngelScript();
		static void ShutdownAngelScript();
		static void ConfigureEngine(asIScriptEngine* engine);
		static void RegisterMathTypes(asIScriptEngine* engine);
		static void RegisterComponents(asIScriptEngine* engine);
		static void RegisterGlobalFunctions(asIScriptEngine* engine);
		static void RegisterScriptInterface(asIScriptEngine* engine);
		static bool BuildModule(const std::filesystem::path& filepath);
		static bool BuildModuleFromFiles(const std::string& moduleName, const std::vector<std::filesystem::path>& files);

	};
}