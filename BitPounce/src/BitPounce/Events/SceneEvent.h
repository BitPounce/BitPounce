#pragma once

#include "Event.h"
#include "BitPounce/Scene/Scene.h"

namespace BitPounce
{
    class SceneLoadedEvent : public Event
	{
	public:
		SceneLoadedEvent(Ref<Scene> scene)
			: m_Scene(scene) {
		}

		inline Ref<Scene> GetScene() const { return m_Scene; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "SceneLoadedEvent: " << m_Scene->name;
			return ss.str();
		}

		EVENT_CLASS_TYPE(SceneLoaded)
		EVENT_CLASS_CATEGORY(EventCategoryScene)
	private:
		Ref<Scene> m_Scene;
	};
}