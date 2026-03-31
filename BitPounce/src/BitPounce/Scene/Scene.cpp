#include "bp_pch.h"
#include "Scene.h"

#include "Components.h"
#include "BitPounce/Renderer/Renderer2D.h"
#include "BitPounce/Scene/Entity.h"

#include <glm/glm.hpp>
/*

namespace BitPounce
{
    inline System* rttr_clone_system(const System* original)
    {
        if (!original) return nullptr;

        rttr::type t = rttr::type::get(*original);
        if (!t.is_valid()) return nullptr;

        rttr::method clone_method = t.get_method("clone");
        if (!clone_method.is_valid()) return nullptr;

        rttr::variant result = clone_method.invoke(original);
        if (!result.is_valid()) return nullptr;

        return result.get_value<System*>();
    }
}*/

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
		entity.AddComponent<ChildrenComponent>();
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name);
		return entity;
    }

    void Scene::DestroyEntity(Entity entity)
	{
		entity.OnDestroy();

		m_Registry.destroy(entity);
	}

    Entity Scene::FindEntityByUUID(UUID uuid) const
    {
        auto view = m_Registry.view<IDComponent>();
    	for (auto entity : view)
    	{
    	    if (view.get<IDComponent>(entity).ID == uuid)
    	        return Entity{ entity, const_cast<Scene*>(this) };
    	}
    	return Entity{};
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

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		for(auto&& sys : other->m_sysManager.m_systems)
		{
			System* cloned = sys->clone();
        	if (cloned)
        	{
        	    newScene->m_sysManager.AddSys_in(cloned);
        	}
        	else
        	{
        	    //BP_CORE_ERROR("Failed to clone system of type {}", /*rttr::type::get(*sys).get_name().data()*/);
        	}
		}

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		other->m_sysManager.CopyComponent(newScene->m_Registry, other->m_Registry, enttMap);
		ECSSystem::CopyComponentBASE<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		ECSSystem::CopyComponentBASE<AngelScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
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
	void ECSSystem::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
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

	template<>
	void ECSSystem::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{
	}

	template<>
	void ECSSystem::OnComponentAdded<AngelScriptComponent>(Entity entity, AngelScriptComponent& component)
	{
	}
}