#include <bp_pch.h>
#include "ScriptEngine.h"

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptarray/scriptarray.h>
#include <scriptdictionary/scriptdictionary.h>
#include <scriptfile/scriptfile.h>
#include <scriptfile/scriptfilesystem.h>
#include <scripthelper/scripthelper.h>
#include <contextmgr/contextmgr.h>
#include <datetime/datetime.h>
#include <weakref/weakref.h>
#include <scripthandle/scripthandle.h>

#include "BitPounce/Core/FileSystem.h"
#include "BitPounce/Core/Logger.h"
#include "BitPounce/Scene/Entity.h"
#include "BitPounce/Scene/Scene.h"
#include "BitPounce/Scene/Components.h"
#include <glm/gtc/type_ptr.hpp>
#include <BitPounce/Core/KeyCode.h>

namespace BitPounce
{
	struct ScriptEngineData
	{
		asIScriptEngine* engine = nullptr;
		std::vector<asIScriptContext*> contextPool;
		std::unordered_map<std::string, asIScriptModule*> modules;
		std::vector<std::filesystem::path> pendingFiles;

		struct EntityRefObj
		{
			int refCount;
			asILockableSharedBool* weakRefFlag;
		};

		// WHYY 2!
		std::unordered_map<Entity*, EntityRefObj> entityRefs;
		std::unordered_map<entt::entity, Entity*> entityToHeap;
	};

	static ScriptEngineData* s_Data = nullptr;

	static Scene* GetCurrentScene()
	{
		asIScriptContext* ctx = asGetActiveContext();
		if (!ctx) return nullptr;
		return static_cast<Scene*>(ctx->GetUserData());
	}

	static void Entity_AddRef(Entity* obj)
	{
		auto it = s_Data->entityRefs.find(obj);
		if (it != s_Data->entityRefs.end())
			++it->second.refCount;
	}

	static void Entity_Release(Entity* obj)
	{
		auto it = s_Data->entityRefs.find(obj);
		if (it == s_Data->entityRefs.end())
			return;

		if (--it->second.refCount == 0)
		{
			if (it->second.weakRefFlag)
			{
				it->second.weakRefFlag->Set(true);
				it->second.weakRefFlag->Release();
			}
			s_Data->entityRefs.erase(it);
			delete obj;
		}
	}

	static asILockableSharedBool*& Entity_GetWeakRefFlag(Entity* obj)
	{
		auto it = s_Data->entityRefs.find(obj);
		if (it == s_Data->entityRefs.end())
		{
			s_Data->entityRefs[obj] = {1, nullptr};
			it = s_Data->entityRefs.find(obj);
		}
		if (!it->second.weakRefFlag)
			it->second.weakRefFlag = asCreateLockableSharedBool();
		return it->second.weakRefFlag;
	}

	static void PrintString(const std::string& str)
	{
		BP_INFO(str);
	}

	static Entity* FindEntityByUUID(uint64_t uuid)
	{
		Scene* scene = GetCurrentScene();
		if (!scene) return nullptr;

		Entity lightweight = scene->FindEntityByUUID(UUID(uuid));
		if (!lightweight) return nullptr;

		return ScriptEngine::GetOrCreateSharedEntity(lightweight.operator entt::entity(), scene);
	}

	static Entity* CreateEntity(const std::string& name)
	{
		Scene* scene = GetCurrentScene();
		if (!scene) return nullptr;

		Entity lightweight = scene->CreateEntity(name);
		return ScriptEngine::GetOrCreateSharedEntity(lightweight.operator entt::entity(), scene);
	}

	// ???
	static float Math_Cos(float v) { return cosf(v); }
	static float Math_Sin(float v) { return sinf(v); }
	static float Math_Tan(float v) { return tanf(v); }
	static float Math_Asin(float v) { return asinf(v); }
	static float Math_Acos(float v) { return acosf(v); }
	static float Math_Atan(float v) { return atanf(v); }
	static float Math_Atan2(float y, float x) { return atan2f(y, x); }
	static float Math_Sqrt(float v) { return sqrtf(v); }
	static float Math_Abs(float v) { return fabsf(v); }
	static float Math_Radians(float deg) { return deg * 3.14159265358979323846f / 180.0f; }
	static float Math_Degrees(float rad) { return rad * 180.0f / 3.14159265358979323846f; }

	static bool Input_GetKeyDown(int v) { return Input::IsKeyPressed((Key)v); }

	static void Vec3_ConstructDefault(glm::vec3* m) { new (m) glm::vec3(0.0f); }
	static void Vec3_ConstructXYZ(glm::vec3* m, float x, float y, float z) { new (m) glm::vec3(x, y, z); }
	static void Vec3_CopyConstruct(const glm::vec3& other, glm::vec3* m) { new (m) glm::vec3(other); }
	static void Vec3_Destructor(glm::vec3* m) {}
	static glm::vec3 Vec3_Add(const glm::vec3& self, const glm::vec3& other) { return self + other; }
	static glm::vec3 Vec3_Sub(const glm::vec3& self, const glm::vec3& other) { return self - other; }
	static glm::vec3 Vec3_MulScalar(const glm::vec3& self, float s) { return self * s; }
	static glm::vec3 Vec3_MulVec(const glm::vec3& self, const glm::vec3& other) { return self * other; }
	static glm::vec3 Vec3_DivScalar(const glm::vec3& self, float s) { return self / s; }
	static glm::vec3 Vec3_DivVec(const glm::vec3& self, const glm::vec3& other) { return self / other; }
	static float Vec3_Length(const glm::vec3& self) { return glm::length(self); }
	static glm::vec3 Vec3_Normalize(const glm::vec3& self) { return glm::normalize(self); }
	static float Vec3_Dot(const glm::vec3& self, const glm::vec3& other) { return glm::dot(self, other); }

	static void Vec4_ConstructDefault(glm::vec4* m) { new (m) glm::vec4(0.0f); }
	static void Vec4_ConstructXYZW(glm::vec4* m, float x, float y, float z, float w) { new (m) glm::vec4(x, y, z, w); }
	static void Vec4_CopyConstruct(const glm::vec4& other, glm::vec4* m) { new (m) glm::vec4(other); }
	static void Vec4_Destructor(glm::vec4* m) {}

	static void Mat4_ConstructDefault(glm::mat4* m) { new (m) glm::mat4(1.0f); }
	static void Mat4_CopyConstruct(glm::mat4* m, const glm::mat4& other) { new (m) glm::mat4(other); }
	static void Mat4_Destructor(glm::mat4* m) {}

	static void TransformComponent_ConstructDefault(TransformComponent* m) { new (m) TransformComponent(); }
	static void TransformComponent_ConstructCopy(TransformComponent* m, const TransformComponent& other) { new (m) TransformComponent(other); }
	static void TransformComponent_Destructor(TransformComponent* m) { m->~TransformComponent(); }
	static void SpriteRendererComponent_ConstructDefault(SpriteRendererComponent* m) { new (m) SpriteRendererComponent(); }
	static void SpriteRendererComponent_ConstructCopy(SpriteRendererComponent* m, const SpriteRendererComponent& other) { new (m) SpriteRendererComponent(other); }
	static void SpriteRendererComponent_Destructor(SpriteRendererComponent* m) { m->~SpriteRendererComponent(); }
	static void CircleRendererComponent_ConstructDefault(CircleRendererComponent* m) { new (m) CircleRendererComponent(); }
	static void CircleRendererComponent_ConstructCopy(CircleRendererComponent* m, const CircleRendererComponent& other) { new (m) CircleRendererComponent(other); }
	static void CircleRendererComponent_Destructor(CircleRendererComponent* m) { m->~CircleRendererComponent(); }
	static void CameraComponent_ConstructDefault(CameraComponent* m) { new (m) CameraComponent(); }
	static void CameraComponent_ConstructCopy(CameraComponent* m, const CameraComponent& other) { new (m) CameraComponent(other); }
	static void CameraComponent_Destructor(CameraComponent* m) { m->~CameraComponent(); }

	static glm::vec3& Vec3_Assign(glm::vec3* m, const glm::vec3& other) { *m = other; return *m; }
	static glm::vec4& Vec4_Assign(glm::vec4* m, const glm::vec4& other) { *m = other; return *m; }
	static glm::mat4& Mat4_Assign(glm::mat4* m, const glm::mat4& other) { *m = other; return *m; }

	// hell of an message callback
	static void MessageCallback(const asSMessageInfo* msg, void* param)
	{
		const char* type = "ERR ";
		if (msg->type == asMSGTYPE_WARNING) type = "WARN";
		else if (msg->type == asMSGTYPE_INFORMATION) type = "INFO";
		BP_CORE_INFO("{} ({}, {}): {}: {}", msg->section, msg->row, msg->col, type, msg->message);
	}

	void ScriptEngine::RegisterMathTypes(asIScriptEngine* engine)
	{
		engine->RegisterObjectType("vec3", sizeof(glm::vec3), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("vec3", "float x", offsetof(glm::vec3, x));
		engine->RegisterObjectProperty("vec3", "float y", offsetof(glm::vec3, y));
		engine->RegisterObjectProperty("vec3", "float z", offsetof(glm::vec3, z));
		engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vec3_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(Vec3_ConstructXYZ), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT, "void f(const vec3 &in)", asFUNCTION(Vec3_CopyConstruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec3", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Vec3_Destructor), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("vec3", "vec3 opAdd(const vec3 &in) const", asFUNCTION(Vec3_Add), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 opSub(const vec3 &in) const", asFUNCTION(Vec3_Sub), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 opMul(float) const", asFUNCTION(Vec3_MulScalar), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 opMul(const vec3 &in) const", asFUNCTION(Vec3_MulVec), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 opDiv(float) const", asFUNCTION(Vec3_DivScalar), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 opDiv(const vec3 &in) const", asFUNCTION(Vec3_DivVec), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "float length() const", asFUNCTION(Vec3_Length), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 normalize() const", asFUNCTION(Vec3_Normalize), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "float dot(const vec3 &in) const", asFUNCTION(Vec3_Dot), asCALL_CDECL_OBJFIRST);
		engine->RegisterObjectMethod("vec3", "vec3 &opAssign(const vec3 &in)", asFUNCTION(Vec3_Assign), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectType("vec4", sizeof(glm::vec4), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("vec4", "float r", offsetof(glm::vec4, r));
		engine->RegisterObjectProperty("vec4", "float g", offsetof(glm::vec4, g));
		engine->RegisterObjectProperty("vec4", "float b", offsetof(glm::vec4, b));
		engine->RegisterObjectProperty("vec4", "float a", offsetof(glm::vec4, a));
		engine->RegisterObjectProperty("vec4", "float x", offsetof(glm::vec4, x));
		engine->RegisterObjectProperty("vec4", "float y", offsetof(glm::vec4, y));
		engine->RegisterObjectProperty("vec4", "float z", offsetof(glm::vec4, z));
		engine->RegisterObjectProperty("vec4", "float w", offsetof(glm::vec4, w));
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vec4_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(Vec4_ConstructXYZW), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT, "void f(const vec4 &in)", asFUNCTION(Vec4_CopyConstruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Vec4_Destructor), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("vec4", "vec4 &opAssign(const vec4 &in)", asFUNCTION(Vec4_Assign), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectType("mat4", sizeof(glm::mat4), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectBehaviour("mat4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Mat4_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("mat4", asBEHAVE_CONSTRUCT, "void f(const mat4 &in)", asFUNCTION(Mat4_CopyConstruct), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("mat4", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Mat4_Destructor), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("mat4", "mat4 &opAssign(const mat4 &in)", asFUNCTION(Mat4_Assign), asCALL_CDECL_OBJLAST);
	}

	void ScriptEngine::RegisterComponents(asIScriptEngine* engine)
	{
		engine->RegisterObjectType("TransformComponent", sizeof(TransformComponent), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("TransformComponent", "vec3 Translation", offsetof(TransformComponent, Translation));
		engine->RegisterObjectProperty("TransformComponent", "vec3 Rotation", offsetof(TransformComponent, Rotation));
		engine->RegisterObjectProperty("TransformComponent", "vec3 Scale", offsetof(TransformComponent, Scale));
		engine->RegisterObjectBehaviour("TransformComponent", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(TransformComponent_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("TransformComponent", asBEHAVE_CONSTRUCT, "void f(const TransformComponent &in)", asFUNCTION(TransformComponent_ConstructCopy), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("TransformComponent", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(TransformComponent_Destructor), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectMethod("TransformComponent", "mat4 GetTransform() const", asMETHOD(TransformComponent, GetTransform), asCALL_THISCALL);
		engine->RegisterObjectMethod("TransformComponent", "mat4 GetLocalTransform() const", asMETHOD(TransformComponent, GetLocalTransform), asCALL_THISCALL);

		engine->RegisterObjectType("SpriteRendererComponent", sizeof(SpriteRendererComponent), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("SpriteRendererComponent", "vec4 Colour", offsetof(SpriteRendererComponent, Colour));
		engine->RegisterObjectBehaviour("SpriteRendererComponent", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(SpriteRendererComponent_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("SpriteRendererComponent", asBEHAVE_CONSTRUCT, "void f(const SpriteRendererComponent &in)", asFUNCTION(SpriteRendererComponent_ConstructCopy), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("SpriteRendererComponent", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(SpriteRendererComponent_Destructor), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectType("CircleRendererComponent", sizeof(CircleRendererComponent), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("CircleRendererComponent", "vec4 Colour", offsetof(CircleRendererComponent, Colour));
		engine->RegisterObjectProperty("CircleRendererComponent", "float Thickness", offsetof(CircleRendererComponent, Thickness));
		engine->RegisterObjectProperty("CircleRendererComponent", "float Fade", offsetof(CircleRendererComponent, Fade));
		engine->RegisterObjectBehaviour("CircleRendererComponent", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CircleRendererComponent_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CircleRendererComponent", asBEHAVE_CONSTRUCT, "void f(const CircleRendererComponent &in)", asFUNCTION(CircleRendererComponent_ConstructCopy), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CircleRendererComponent", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CircleRendererComponent_Destructor), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectType("CameraComponent", sizeof(CameraComponent), asOBJ_VALUE | asOBJ_APP_CLASS_CAK);
		engine->RegisterObjectProperty("CameraComponent", "bool Primary", offsetof(CameraComponent, Primary));
		engine->RegisterObjectProperty("CameraComponent", "bool FixedAspectRatio", offsetof(CameraComponent, FixedAspectRatio));
		engine->RegisterObjectBehaviour("CameraComponent", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CameraComponent_ConstructDefault), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CameraComponent", asBEHAVE_CONSTRUCT, "void f(const CameraComponent &in)", asFUNCTION(CameraComponent_ConstructCopy), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("CameraComponent", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CameraComponent_Destructor), asCALL_CDECL_OBJLAST);
	}

	void ScriptEngine::RegisterGlobalFunctions(asIScriptEngine* engine)
	{
		engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(PrintString), asCALL_CDECL);
		engine->RegisterGlobalFunction("Entity@ FindEntityByUUID(uint64)", asFUNCTION(FindEntityByUUID), asCALL_CDECL);
		engine->RegisterGlobalFunction("Entity@ CreateEntity(const string &in)", asFUNCTION(CreateEntity), asCALL_CDECL);
	}

	void ScriptEngine::RegisterScriptInterface(asIScriptEngine* engine)
	{
		engine->RegisterInterface("IScript");
		engine->RegisterInterfaceMethod("IScript", "void OnCreate(Entity@)");
		engine->RegisterInterfaceMethod("IScript", "void OnUpdate(float)");
		engine->RegisterInterfaceMethod("IScript", "void OnDestroy()");
	}

	void ScriptEngine::ConfigureEngine(asIScriptEngine* engine)
	{
		engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

		// Standard add‑ons
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterStdStringUtils(engine);
		RegisterScriptDictionary(engine);
		RegisterScriptDateTime(engine);
		RegisterScriptFile(engine);
		RegisterScriptWeakRef(engine);
		RegisterScriptHandle(engine);
		RegisterScriptFileSystem(engine);
		RegisterExceptionRoutines(engine);

		RegisterMathTypes(engine);

		engine->RegisterObjectType("Entity", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("Entity", asBEHAVE_ADDREF, "void f()", asFUNCTION(Entity_AddRef), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("Entity", asBEHAVE_RELEASE, "void f()", asFUNCTION(Entity_Release), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectBehaviour("Entity", asBEHAVE_GET_WEAKREF_FLAG, "int& f()", asFUNCTION(Entity_GetWeakRefFlag), asCALL_CDECL_OBJLAST);

		engine->RegisterObjectMethod("Entity", "string GetName()", asMETHOD(Entity, GetName), asCALL_THISCALL);
		engine->RegisterObjectMethod("Entity", "uint64 GetUUID()", asMETHOD(Entity, GetUUID), asCALL_THISCALL);


		RegisterComponents(engine);
		RegisterGlobalFunctions(engine);
		RegisterScriptInterface(engine);

		engine->RegisterEnum("Key");
		#define BP_KEY(caps, name)	engine->RegisterEnumValue("Key", #name, (int)Key::name);
		#include "BitPounce/Core/KeyList.def"
		#undef BP_KEY

		engine->RegisterGlobalFunction("bool GetKeyDown(Key)", asFUNCTION(Input_GetKeyDown), asCALL_CDECL);

		engine->RegisterGlobalFunction("float cos(float)", asFUNCTION(Math_Cos), asCALL_CDECL);
		engine->RegisterGlobalFunction("float sin(float)", asFUNCTION(Math_Sin), asCALL_CDECL);
		engine->RegisterGlobalFunction("float tan(float)", asFUNCTION(Math_Tan), asCALL_CDECL);
		engine->RegisterGlobalFunction("float asin(float)", asFUNCTION(Math_Asin), asCALL_CDECL);
		engine->RegisterGlobalFunction("float acos(float)", asFUNCTION(Math_Acos), asCALL_CDECL);
		engine->RegisterGlobalFunction("float atan(float)", asFUNCTION(Math_Atan), asCALL_CDECL);
		engine->RegisterGlobalFunction("float atan2(float, float)", asFUNCTION(Math_Atan2), asCALL_CDECL);
		engine->RegisterGlobalFunction("float sqrt(float)", asFUNCTION(Math_Sqrt), asCALL_CDECL);
		engine->RegisterGlobalFunction("float abs(float)", asFUNCTION(Math_Abs), asCALL_CDECL);
		engine->RegisterGlobalFunction("float radians(float)", asFUNCTION(Math_Radians), asCALL_CDECL);
		engine->RegisterGlobalFunction("float degrees(float)", asFUNCTION(Math_Degrees), asCALL_CDECL);

		engine->RegisterObjectMethod("Entity", "TransformComponent& GetTransform()", asMETHOD(Entity, GetTransform), asCALL_THISCALL);
		engine->RegisterObjectMethod("Entity", "SpriteRendererComponent& GetSpriteRenderer()", asMETHODPR(Entity, GetComponent, (), SpriteRendererComponent&), asCALL_THISCALL);
		engine->RegisterObjectMethod("Entity", "CircleRendererComponent& GetCircleRenderer()", asMETHODPR(Entity, GetComponent, (), CircleRendererComponent&), asCALL_THISCALL);
		engine->RegisterObjectMethod("Entity", "CameraComponent& GetCamera()", asMETHODPR(Entity, GetComponent, (), CameraComponent&), asCALL_THISCALL);

		
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
		s_Data = nullptr;
	}

	void ScriptEngine::InitAngelScript()
	{
		s_Data->engine = asCreateScriptEngine();
		ConfigureEngine(s_Data->engine);
	}

	void ScriptEngine::ShutdownAngelScript()
	{
		if (s_Data->engine)
		{
			s_Data->engine->ShutDownAndRelease();
			s_Data->engine = nullptr;
		}
		for (auto ctx : s_Data->contextPool)
			ctx->Release();
		s_Data->contextPool.clear();
		s_Data->modules.clear();
		s_Data->pendingFiles.clear();

		for (auto& pair : s_Data->entityRefs)
		{
			if (pair.second.weakRefFlag)
				pair.second.weakRefFlag->Release();
			delete pair.first;
		}
		s_Data->entityRefs.clear();
		s_Data->entityToHeap.clear();
	}


	Entity* ScriptEngine::GetOrCreateSharedEntity(entt::entity handle, Scene* scene)
	{
		auto it = s_Data->entityToHeap.find(handle);
		if (it != s_Data->entityToHeap.end())
			return it->second;

		Entity lightweight{ handle, scene };
		Entity* heapEntity = new Entity(lightweight);
		s_Data->entityToHeap[handle] = heapEntity;
		s_Data->entityRefs[heapEntity] = {1, nullptr};
		return heapEntity;
	}

	void ScriptEngine::InvalidateSharedEntity(entt::entity handle)
	{
		auto it = s_Data->entityToHeap.find(handle);
		if (it == s_Data->entityToHeap.end())
			return;

		Entity* heapEntity = it->second;
		auto refIt = s_Data->entityRefs.find(heapEntity);
		if (refIt != s_Data->entityRefs.end() && refIt->second.weakRefFlag)
		{
			refIt->second.weakRefFlag->Set(true);
		}
		s_Data->entityToHeap.erase(it);
	}

	void ScriptEngine::Build()
	{
		if (s_Data->pendingFiles.empty())
		{
			BP_CORE_WARN("No script files to build.");
			return;
		}
		if (BuildModuleFromFiles("scripts", s_Data->pendingFiles))
		{
			asIScriptModule* mod = s_Data->engine->GetModule("scripts");
			if (mod)
			{
				asIScriptFunction* mainFunc = mod->GetFunctionByDecl("int main()");
				if (mainFunc)
				{
					asIScriptContext* ctx = s_Data->engine->CreateContext();
					ctx->Prepare(mainFunc);
					int r = ctx->Execute();
					if (r != asEXECUTION_FINISHED)
						BP_CORE_ERROR("Script main() execution failed");
					ctx->Release();
				}
			}
		}
		else
		{
			BP_CORE_ERROR("Failed to build script module 'scripts'");
		}
	}

	void ScriptEngine::BuildScriptsDirs(std::vector<std::filesystem::path> dirs)
	{
		for (const auto& dir : dirs)
		{
			if (!std::filesystem::exists(dir)) continue;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".as")
					s_Data->pendingFiles.push_back(entry.path());
			}
		}
	}

	void ScriptEngine::BuildScriptsFiles(std::vector<std::filesystem::path> files)
	{
		for (const auto& file : files)
		{
			if (std::filesystem::exists(file) && file.extension() == ".as")
				s_Data->pendingFiles.push_back(file);
			else
				BP_CORE_WARN("Script file does not exist or wrong extension: {}", file.string());
		}
	}

	void ScriptEngine::BuildScripts(const std::vector<std::filesystem::path>& dirs)
	{
		std::vector<std::filesystem::path> files;
		for (const auto& dir : dirs)
		{
			if (!std::filesystem::exists(dir)) continue;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".as")
					files.push_back(entry.path());
			}
		}
		if (BuildModuleFromFiles("scripts", files))
		{
			asIScriptModule* mod = s_Data->engine->GetModule("scripts");
			if (mod)
			{
				asIScriptFunction* mainFunc = mod->GetFunctionByDecl("int main()");
				if (mainFunc)
				{
					asIScriptContext* ctx = s_Data->engine->CreateContext();
					ctx->Prepare(mainFunc);
					int r = ctx->Execute();
					if (r != asEXECUTION_FINISHED)
						BP_CORE_ERROR("Script main() execution failed");
					ctx->Release();
				}
			}
		}
	}

	bool ScriptEngine::BuildModuleFromFiles(const std::string& moduleName, const std::vector<std::filesystem::path>& files)
	{
		if (files.empty()) return false;

		CScriptBuilder builder;
		int r = builder.StartNewModule(s_Data->engine, moduleName.c_str());
		if (r < 0)
		{
			BP_CORE_ERROR("Failed to start module '{}'", moduleName);
			return false;
		}

		for (const auto& file : files)
		{
			BufferBase buffer = FileSystem::LoadFile(file);
			if (buffer.Size == 0)
			{
				BP_CORE_ERROR("Failed to load script file: {}", file.string());
				return false;
			}

			std::string sectionName = file.filename().string();
			r = builder.AddSectionFromMemory(sectionName.c_str(), buffer.As<const char>(), buffer.Size);
			if (r < 0)
			{
				BP_CORE_ERROR("Failed to add section '{}' from file: {}", sectionName, file.string());
				return false;
			}
		}

		r = builder.BuildModule();
		if (r < 0)
		{
			BP_CORE_ERROR("Failed to build module '{}'", moduleName);
			return false;
		}

		asIScriptModule* mod = s_Data->engine->GetModule(moduleName.c_str());
		if (!mod) return false;

		s_Data->modules[moduleName] = mod;
		BP_CORE_INFO("Built module '{}' with {} files", moduleName, files.size());
		return true;
	}

	asIScriptFunction* ScriptEngine::GetFunction(const std::string& moduleName, const std::string& funcDecl)
	{
		auto it = s_Data->modules.find(moduleName);
		if (it == s_Data->modules.end()) return nullptr;
		return it->second->GetFunctionByDecl(funcDecl.c_str());
	}

	asIScriptEngine* ScriptEngine::GetEngine()
	{
		return s_Data ? s_Data->engine : nullptr;
	}

}

// this is the bigest hackest code ever!
// why, just why.
// TODO make better!