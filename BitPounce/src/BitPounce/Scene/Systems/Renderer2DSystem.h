#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include <BitPounce/Renderer/Renderer2D.h>
#include "../Components.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace BitPounce
{
	class Renderer2DSystem : public ECSSystem
	{
	public:
		Renderer2DSystem() {};

		virtual void OnDraw(Timestep& ts) override 
		{
			auto campar = m_Scene->GetActiveCamera();
			if(!campar.first) { return; }
			auto mainCamera = campar.first->Camera;
			auto cameraTransform = campar.second->Transform;

			Renderer2D::BeginScene(mainCamera.GetProjection(), cameraTransform);

			auto group = m_Scene->GetRegistry(*this).group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto&& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Colour);
			}

			Renderer2D::EndScene();
		};


		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			if (entity.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer"))
				{
					auto& src = entity.GetComponent<SpriteRendererComponent>();
					ImGui::ColorEdit4("Colour", glm::value_ptr(src.Colour));
					ImGui::TreePop();
				}
			}
		}

	};
}