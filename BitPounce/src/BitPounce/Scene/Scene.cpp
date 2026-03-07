#include "bp_pch.h"
#include "Scene.h"

#include "Components.h"
#include "BitPounce/Renderer/Renderer2D.h"

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

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
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
}