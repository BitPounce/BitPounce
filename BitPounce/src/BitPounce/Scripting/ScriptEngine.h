
#include <BitPounce/Core/Base.h>
#include <vector>
#include <filesystem>

namespace BitPounce
{
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static void Build();
		static void BuildScriptsDirs(std::vector<std::filesystem::path> dirs);
		static void BuildScriptsFiles(std::vector<std::filesystem::path> files);
	private:
		static void InitAngelScript();
		static void ShutdownAngelScript();
	};
}