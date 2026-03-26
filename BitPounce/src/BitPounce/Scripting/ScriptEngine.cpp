#include <bp_pch.h>
#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptarray/scriptarray.h>
#include <scriptdictionary/scriptdictionary.h>
#include <scriptfile/scriptfile.h>
#include <scriptfile/scriptfilesystem.h>
#include <scripthelper/scripthelper.h>
#include <debugger/debugger.h>
#include <contextmgr/contextmgr.h>
#include <datetime/datetime.h>
#include <scriptsocket/scriptsocket.h>
#include "ScriptEngine.h"
#include <BitPounce/Core/FileSystem.h>

namespace BitPounce
{
    struct ScriptEngineData
    {
        asIScriptEngine* engine;
        CContextMgr* ctxMgr;
        std::vector<asIScriptContext*> g_ctxPool;
		std::vector<std::filesystem::path> FilePaths;
    };

    static ScriptEngineData* s_Data = nullptr;

    void MessageCallback(const asSMessageInfo *msg, void *param)
    {
    	const char *type = "ERR ";
    	if( msg->type == asMSGTYPE_WARNING ) 
    		type = "WARN";
    	else if( msg->type == asMSGTYPE_INFORMATION ) 
    		type = "INFO";

    	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    }

     // This is the to-string callback for the string type
    std::string StringToString(void *obj, int /* expandMembers */, CDebugger * /* dbg */)
    {
    	// We know the received object is a string
    	std::string *val = reinterpret_cast<std::string*>(obj);

    	// Format the output string
    	// TODO: Should convert non-readable characters to escape sequences
    	std::stringstream s;
    	s << "(len=" << val->length() << ") \"";
    	if( val->length() < 20 )
    		s << *val << "\"";
    	else
    		s << val->substr(0, 20) << "...";

    	return s.str();
    }

    // This is the to-string callback for the array type
    // This is generic and will take care of all template instances based on the array template
    std::string ArrayToString(void *obj, int expandMembers, CDebugger *dbg)
    {
    	CScriptArray *arr = reinterpret_cast<CScriptArray*>(obj);

    	std::stringstream s;
    	s << "(len=" << arr->GetSize() << ")";
    
    	if( expandMembers > 0 )
    	{
    		s << " [";
    		for( asUINT n = 0; n < arr->GetSize(); n++ )
    		{
    			s << dbg->ToString(arr->At(n), arr->GetElementTypeId(), expandMembers - 1, arr->GetArrayObjectType()->GetEngine());
    			if( n < arr->GetSize()-1 )
    				s << ", ";
    		}
    		s << "]";
    	}

    	return s.str();
    }

    // This is the to-string callback for the dictionary type
    std::string DictionaryToString(void *obj, int expandMembers, CDebugger *dbg)
    {
    	CScriptDictionary *dic = reinterpret_cast<CScriptDictionary*>(obj);
    
    	std::stringstream s;
    	s << "(len=" << dic->GetSize() << ")";
    
    	if( expandMembers > 0 )
    	{
    		s << " [";
    		asUINT n = 0;
    		for( CScriptDictionary::CIterator it = dic->begin(); it != dic->end(); it++, n++ )
    		{
    			s << "[" << it.GetKey() << "] = ";

    			// Get the type and address of the value
    			const void *val = it.GetAddressOfValue();
    			int typeId = it.GetTypeId();

    			// Use the engine from the currently active context (if none is active, the debugger
    			// will use the engine held inside it by default, but in an environment where there
    			// multiple engines this might not be the correct instance).
    			asIScriptContext *ctx = asGetActiveContext();

    			s << dbg->ToString(const_cast<void*>(val), typeId, expandMembers - 1, ctx ? ctx->GetEngine() : 0);
            
    			if( n < dic->GetSize() - 1 )
    				s << ", ";
    		}
    		s << "]";
    	}
    
    	return s.str();
    }

    // This is the to-string callback for the dictionary type
    std::string DateTimeToString(void *obj, int expandMembers, CDebugger *dbg)
    {
    	CDateTime *dt = reinterpret_cast<CDateTime*>(obj);
    
    	std::stringstream s;
    	s << "{" << dt->getYear() << "-" << dt->getMonth() << "-" << dt->getDay() << " ";
    	s << dt->getHour() << ":" << dt->getMinute() << ":" << dt->getSecond() << "}";
    
    	return s.str(); 
    }

    void PrintString(const std::string &str)
    {
        BP_INFO(str);
    }

    void ConfigureEngine(asIScriptEngine *engine)
    {
        engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

        RegisterStdString(engine);
	    RegisterScriptArray(engine, false);
	    RegisterStdStringUtils(engine);
	    RegisterScriptDictionary(engine);
	    RegisterScriptDateTime(engine);
	    RegisterScriptFile(engine);
	    RegisterScriptFileSystem(engine);
	    RegisterExceptionRoutines(engine);
	    RegisterScriptSocket(engine);

        engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(PrintString), asCALL_CDECL);

        s_Data->ctxMgr = new CContextMgr();
        s_Data->ctxMgr->RegisterCoRoutineSupport(engine);
    }

    

    void ScriptEngine::Init()
    {
        s_Data = new ScriptEngineData();
        InitAngelScript();
    }

    void ScriptEngine::Shutdown()
    {
        ShutdownAngelScript();
        delete s_Data;
    }

    void ScriptEngine::Build()
    {
		s_Data->engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		CScriptBuilder builder;

		builder.StartNewModule(s_Data->engine, "script");
		for(auto&& filepath : s_Data->FilePaths)
		{
			BufferBase buffer = FileSystem::LoadFile(filepath);
			
			builder.AddSectionFromMemory(filepath.filename().string().c_str(), buffer.As<const char>(), buffer.Size);
		}

		builder.BuildModule();
		
		asIScriptModule* mod = s_Data->engine->GetModule("script", asGM_ONLY_IF_EXISTS);

		asIScriptFunction* func = mod->GetFunctionByDecl("int main()");

		int r = mod->ResetGlobalVars(0);
		asIScriptContext *ctx = s_Data->ctxMgr->AddContext(s_Data->engine, func, true);

		while( s_Data->ctxMgr->ExecuteScripts() );
    }

    void ScriptEngine::BuildScriptsDirs(std::vector<std::filesystem::path> dirs)
    {
		for(auto&& dirPath : dirs)
		{
			std::vector<std::filesystem::path> dirdirs = std::vector<std::filesystem::path>();
			std::vector<std::filesystem::path> dirfiles = std::vector<std::filesystem::path>();

			for(auto&& directoryEntry : std::filesystem::directory_iterator(dirPath))
			{
				if (directoryEntry.is_directory())
				{
					dirdirs.push_back(directoryEntry.path());
				}
				else if (directoryEntry.is_regular_file() && directoryEntry.path().extension() == ".as")
				{
					dirfiles.push_back(directoryEntry.path());
				}
			}

			BuildScriptsDirs(dirdirs);
			BuildScriptsFiles(dirfiles);
		}
    }

    void ScriptEngine::BuildScriptsFiles(std::vector<std::filesystem::path> files)
    {
		for(auto&& file : files)
		{
			s_Data->FilePaths.push_back(file);
		}
    }

    void ScriptEngine::InitAngelScript()
    {
        s_Data->engine = asCreateScriptEngine();
        ConfigureEngine(s_Data->engine);
    }
    void ScriptEngine::ShutdownAngelScript()
    {
    }

    
}
