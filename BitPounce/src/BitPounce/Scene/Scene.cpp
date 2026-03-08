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

    void Scene::OnEvent(Event &e)
    {
		m_sysManager.OnEvent(e);

		EventDispatcher dis = EventDispatcher(e);
		dis.Dispatch<WindowResizeEvent>(BP_BIND_EVENT_FN(OnResize));

        return;
    }

    void Scene::OnEditorPropImguiDraw(Entity &entity)
    {
		m_sysManager.OnEditorPropImguiDraw(entity);
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
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
    bool Scene::OnResize(WindowResizeEvent &e)
    {
        
    }
}