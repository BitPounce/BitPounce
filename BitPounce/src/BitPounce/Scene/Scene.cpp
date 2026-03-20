#include "bp_pch.h"
#include "Scene.h"

#include "Components.h"
#include "BitPounce/Renderer/Renderer2D.h"
#include "BitPounce/Scene/Entity.h"

#include <glm/glm.hpp>

namespace BitPounce {


	Scene::Scene(const std::string& name)
	{
		m_sysManager = ECSSystemManager();
		m_sysManager.m_Scene = this;
		this->name = name;
		
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name)
    {
        Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
    }

    void Scene::DestroyEntity(Entity entity)
	{
		entity.OnDestroy();

		m_Registry.destroy(entity);
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
			{
				cameraComponent.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			}
				
		}
	}

    void Scene::OnRuntimeStart()
    {
		m_sysManager.OnRuntimeStart();
    }

    void Scene::OnRuntimeStop()
    {
		m_sysManager.OnRuntimeStop();
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
		OnUpdate(ts);
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera &camera)
    {
		m_sysManager.OnDrawEditor(ts, camera);
	}



	void Scene::AddComponentPopupImguiDraw(Entity &ent)
	{
		m_sysManager.AddComponentPopupImguiDraw(ent);
	}

	void Scene::AddedAllSys()
	{
		m_sysManager.Start();
	}

	void Scene::Serialize(nlohmann::json &json) 
	{
		m_sysManager.Serialize(json);
	}

	void Scene::Deserialize(nlohmann::json& json)
	{
		m_sysManager.Deserialize(json);
	}

    void Scene::RemoveAll()
    {
		m_Registry.clear();
    }

    std::pair<CameraComponent *, TransformComponent *> Scene::GetActiveCamera() 
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent>();

		for (auto entity : view) {
			auto &camera = view.get<CameraComponent>(entity);
			auto &transform = view.get<TransformComponent>(entity);

			if (camera.Primary)
				return {&camera, &transform};
	  	}

		return {nullptr, nullptr};
		
	}
	bool Scene::OnResize(WindowResizeEvent &e)
	{
		
	}

	template<typename T>
	void ECSSystem::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_Scene->m_ViewportWidth, m_Scene->m_ViewportHeight);
	}

	template<>
	void ECSSystem::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}
}