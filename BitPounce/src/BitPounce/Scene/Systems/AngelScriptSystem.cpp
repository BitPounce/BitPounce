#include <bp_pch.h>
#include "AngelScriptSystem.h"
#include <angelscript.h>
#include <imgui.h>
#include <BitPounce/ImGui/ImGuiUtils.h>
#include <weakref/weakref.h>
#include <BitPounce/Scripting/ScriptEngine.h>

namespace BitPounce
{
    void AngelScriptSystem::Serialize(nlohmann::json &json)
    {
		auto& registry = m_Scene->GetRegistry(*this);
		
		auto scriptView = registry.view<AngelScriptComponent>();
		for (auto entity : scriptView)
		{
		    auto& script = scriptView.get<AngelScriptComponent>(entity);
		
		    nlohmann::json scriptJson;
		    scriptJson["ScriptClassName"] = script.ScriptClassName;
		
		    for (auto& ent : json["Entities"])
		    {
		        if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
		        {
		            ent["AngelScript"] = scriptJson;
		            break;
		        }
		    }
		}
    }

    void AngelScriptSystem::Deserialize(nlohmann::json &json)
    {
		auto& registry = m_Scene->GetRegistry(*this);
		
		for (auto& entJson : json["Entities"])
		{
		    if (!entJson.contains("entityID"))
		        continue;
		
		    uint32_t id = entJson["entityID"].get<uint32_t>();
		    entt::entity targetEntity = entt::null;
		
		    auto view = registry.view<TagComponent>();
		    for (auto entity : view)
		    {
		        Entity e{ entity, m_Scene };
		        if ((uint32_t)e == id)
		        {
		            targetEntity = entity;
		            break;
		        }
		    }
		
		    if (targetEntity == entt::null)
		        continue;
		
		    Entity entity{ targetEntity, m_Scene };

			if (entJson.contains("AngelScript"))
		    {
		        AngelScriptComponent comp;
		        auto& scriptJson = entJson["AngelScript"];
		        comp.ScriptClassName = scriptJson["ScriptClassName"].get<std::string>();
		        entity.AddComponent<AngelScriptComponent>(comp);
		    }
		}
    }

    void AngelScriptSystem::OnRuntimeStart()
	{
	}

	void AngelScriptSystem::OnUpdate(Timestep& ts)
	{
		asIScriptEngine* engine = ScriptEngine::GetEngine();
		if (!engine) return;

		asIScriptModule* module = engine->GetModule("scripts");
		if (!module) return;

		auto& registry = m_Scene->GetRegistry(*this);
		registry.view<AngelScriptComponent>().each([&](auto entity, AngelScriptComponent& asc) {
		if (!asc.ScriptObject)
		{
			auto& info = m_ClassCache[asc.ScriptClassName];
			if (info.typeInfo == nullptr)
			{
				asITypeInfo* type = module->GetTypeInfoByName(asc.ScriptClassName.c_str());
				if (!type)
				{
					BP_CORE_ERROR("Script class '{}' not found", asc.ScriptClassName);
					return;
				}
				info.typeInfo = type;
				info.onCreateFunc = type->GetMethodByDecl("void OnCreate(Entity@)");
				info.onUpdateFunc = type->GetMethodByDecl("void OnUpdate(float)");
				info.onDestroyFunc = type->GetMethodByDecl("void OnDestroy()");
			}
			asIScriptObject* obj = static_cast<asIScriptObject*>(
				engine->CreateScriptObject(info.typeInfo));
			if (!obj)
			{
				BP_CORE_ERROR("Failed to create script object for '{}'", asc.ScriptClassName);
				return;
			}
			asc.ScriptObject = obj;
			Entity* heapEntity = ScriptEngine::GetOrCreateSharedEntity(entity, m_Scene);
			if (info.onCreateFunc)
			{
				asIScriptContext* ctx = engine->CreateContext();
				ctx->Prepare(info.onCreateFunc);
				ctx->SetObject(obj);
				ctx->SetArgObject(0, heapEntity);
				int r = ctx->Execute();
				if (r != asEXECUTION_FINISHED)
				{
					if (r == asEXECUTION_EXCEPTION)
					{
						asIScriptFunction* func = ctx->GetExceptionFunction();
						std::string funcName = func ? func->GetDeclaration() : "unknown";
						std::string exception = ctx->GetExceptionString();
						BP_CORE_ERROR("OnCreate threw exception in '{}': {}", funcName, exception);
					}
					else
					{
						BP_CORE_ERROR("OnCreate execution failed for '{}' (error {})", asc.ScriptClassName, r);
					}
				}
				ctx->Release();
				BP_CORE_INFO("Ran OnCreate for {} (class {})", heapEntity->GetName(), asc.ScriptClassName);
			}
		}
		if (asc.ScriptObject)
		{
			auto& info = m_ClassCache[asc.ScriptClassName];
			if (info.onUpdateFunc)
			{
				asIScriptContext* ctx = engine->CreateContext();
				int prep = ctx->Prepare(info.onUpdateFunc);
				if (prep < 0)
				{
					BP_CORE_ERROR("Prepare OnUpdate for '{}' failed with code {}", asc.ScriptClassName, prep);
					ctx->Release();
					return;
				}
				ctx->SetObject(asc.ScriptObject);
				ctx->SetArgFloat(0, ts);
				int r = ctx->Execute();
				if (r != asEXECUTION_FINISHED)
				{
					if (r == asEXECUTION_EXCEPTION)
					{
						asIScriptFunction* func = ctx->GetExceptionFunction();
						std::string funcName = func ? func->GetDeclaration() : "unknown";
						std::string exception = ctx->GetExceptionString();
						BP_CORE_ERROR("OnUpdate threw exception in '{}': {}", funcName, exception);
					}
					else
					{
						BP_CORE_ERROR("OnUpdate execution failed for '{}' (error {})", asc.ScriptClassName, r);
					}
				}
				ctx->Release();
			}
		}});
	}

	void AngelScriptSystem::OnEditorPropImguiDraw(Entity& entity)
	{
		ImGuiUtils::DrawComponent<AngelScriptComponent>("Angel Script", entity, [](AngelScriptComponent& comp)
		{
			char buffer[1024] = {0};
			std::strcpy(buffer, comp.ScriptClassName.data());
			if (ImGui::InputText("ScriptClassName", buffer, 1024))
			{
				comp.ScriptClassName = std::string(buffer);
			}
		});
	}

	void AngelScriptSystem::OnRuntimeStop()
	{
		asIScriptEngine* engine = ScriptEngine::GetEngine();
		if (!engine) return;

		auto& registry = m_Scene->GetRegistry(*this);
		registry.view<AngelScriptComponent>().each([&](entt::entity entity, AngelScriptComponent& asc) {
		if (asc.ScriptObject)
		{
			auto& info = m_ClassCache[asc.ScriptClassName];
			if (info.onDestroyFunc)
			{
				asIScriptContext* ctx = engine->CreateContext();
				ctx->Prepare(info.onDestroyFunc);
				ctx->SetObject(asc.ScriptObject);
				int r = ctx->Execute();
				if (r != asEXECUTION_FINISHED)
				{
					if (r == asEXECUTION_EXCEPTION)
					{
						asIScriptFunction* func = ctx->GetExceptionFunction();
						std::string funcName = func ? func->GetDeclaration() : "unknown";
						std::string exception = ctx->GetExceptionString();
						BP_CORE_ERROR("OnDestroy threw exception in '{}': {}", funcName, exception);
					}
					else
					{
						BP_CORE_ERROR("OnDestroy execution failed for '{}' (error {})", asc.ScriptClassName, r);
					}
				}
				ctx->Release();
			}
			engine->ReleaseScriptObject(asc.ScriptObject, info.typeInfo);
			asc.ScriptObject = nullptr;
		}});

		std::vector<entt::entity> handles;
		registry.view<AngelScriptComponent>().each([&](entt::entity entity, const AngelScriptComponent& asc) {
		if (entity != entt::null)
			handles.push_back(entity);
		});
		for (entt::entity handle : handles)
			ScriptEngine::InvalidateSharedEntity(handle);

		registry.clear<AngelScriptComponent>();
		m_ClassCache.clear();
	}

	void AngelScriptSystem::AddComponentPopupImguiDraw(Entity& ent)
	{
		if (ImGui::MenuItem("AngelScript"))
		{
			AngelScriptComponent asc;
			asc.ScriptClassName = "";
			ent.AddComponent<AngelScriptComponent>(asc);
			ImGui::CloseCurrentPopup();
		}
	}
}