#include "bp_pch.h"
#include "Scene.h"

#include "Components.h"
#include "BitPounce/Renderer/Renderer2D.h"
#include "BitPounce/Scene/Entity.h"

#include <glm/glm.hpp>

namespace BitPounce {


	Scene::Scene()
	{
		m_sysManager = ECSSystemManager();
		m_sysManager.m_Scene = this;
		
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		auto e = Entity{ m_Registry.create(), this };
		e.AddComponent<TransformComponent>();
		e.AddComponent<TagComponent>(name);
		return e;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_sysManager.OnUpdate(ts);
		m_sysManager.OnDraw(ts);


	}

    void Scene::AddedAllSys()
    {
		m_sysManager.Start();
    }
    std::pair<CameraComponent*, TransformComponent*> Scene::GetActiveCamera()
	{
	    auto view = m_Registry.view<TransformComponent, CameraComponent>();

	    for (auto entity : view)
	    {
	        auto& camera = view.get<CameraComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);

	        if (camera.Primary)
	            return {&camera, &transform};
	    }

	    return {nullptr, nullptr};
	}
}