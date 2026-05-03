#pragma once

#include "Scene.h"
#include "SceneSerializerUtils.h"
#include <BitPouncePack/BitPouncePack.h>

namespace BitPounce 
{

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		virtual void Serialize(const std::string& filepath);
		virtual BitPouncePack::PackScene SerializeRuntime();

		virtual bool Deserialize(const std::string& filepath);
		virtual bool DeserializeRuntime(const BitPouncePack::PackScene& packScene);
	protected:
		virtual nlohmann::json SceneToJson();
		virtual bool Deserialize_IN(nlohmann::json json);
		Ref<Scene> m_Scene;
	};

}