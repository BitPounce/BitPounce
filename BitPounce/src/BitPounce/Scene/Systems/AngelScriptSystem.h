#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include "BitPounce/Scripting/ScriptEngine.h"
#include "BitPounce/Scene/Components.h"

struct asITypeInfo;
struct asIScriptFunction;

namespace BitPounce
{
	class AngelScriptSystem : public ECSSystem
	{
	public:
		virtual void Serialize(nlohmann::json& json) override;
		virtual void Deserialize(nlohmann::json& json) override;
		virtual void OnRuntimeStart() override;
		virtual void OnUpdate(Timestep& ts) override;
		virtual void OnEditorPropImguiDraw(Entity& entity) override;
		virtual void OnRuntimeStop() override;
		virtual void AddComponentPopupImguiDraw(Entity& ent) override;

		virtual System* clone() const override {
			return new AngelScriptSystem(*this);
		}

	private:
		struct ScriptClassInfo
		{
			asITypeInfo* typeInfo = nullptr;
			asIScriptFunction* onCreateFunc = nullptr;
			asIScriptFunction* onUpdateFunc = nullptr;
			asIScriptFunction* onDestroyFunc = nullptr;
		};

		std::unordered_map<std::string, ScriptClassInfo> m_ClassCache;
	};
}