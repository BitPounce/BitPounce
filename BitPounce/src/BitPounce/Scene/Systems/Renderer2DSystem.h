#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include <BitPounce/Renderer/Renderer2D.h>
#include "../Components.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "BitPounce/ImGui/ImGuiUtils.h"

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
			auto cameraTransform = campar.second->GetTransform();

			Renderer2D::BeginScene(mainCamera.GetProjection(), cameraTransform);

			auto group = m_Scene->GetRegistry(*this).group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto&& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour);
			}

			Renderer2D::EndScene();
		};

		virtual void AddComponentPopupImguiDraw(Entity& ent) override
		{
			if (ImGui::MenuItem("Sprite Renderer"))
			{
				ent.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}
		};

		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			ImGuiUtils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
			{
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
			});
		}

	};
}