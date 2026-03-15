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

			Draw(mainCamera.GetProjection() * glm::inverse(cameraTransform));
		};

		virtual void OnDrawEditor(Timestep& ts, EditorCamera& cam) override 
		{
			Draw(cam.GetViewProjection());
		}

		void Draw(const glm::mat4& cam)
		{
			Renderer2D::BeginScene(cam);

			auto group = m_Scene->GetRegistry(*this).group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto&& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour);
			}

			Renderer2D::EndScene();
		}

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

		virtual void Serialize(nlohmann::json& json)
		{
			auto group = m_Scene->GetRegistry(*this).group<SpriteRendererComponent>();
			for (auto entity : group)
			{
				

				auto&& sprite = group.get<SpriteRendererComponent>(entity);

				nlohmann::json spriteRendererComponent = nlohmann::json();
				spriteRendererComponent["Colour"] = sprite.Colour;

				nlohmann::json* entjson = nullptr;
				for(auto& ent : json["Entities"])
				{
					auto entityID = ent["entityID"];
					if(entityID.get<uint32_t>() == (uint32_t)entity)
					{
						ent["SpriteRenderer"] = spriteRendererComponent;
						break;
					}
				}
			}
			
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			for (auto& entJson : json["Entities"])
			{
				if (!entJson.contains("entityID") || !entJson.contains("SpriteRenderer"))
					continue;

				uint32_t id = entJson["entityID"].get<uint32_t>();

				entt::entity targetEntity = entt::null;
				auto view = registry.view<TagComponent>(); 
				for (auto entity : view)
				{
					Entity e{ entity, m_Scene };
					if ((uint32_t)e == id)
					{
						targetEntity = entity;
						break;
					}
				}

				if (targetEntity == entt::null)
					continue;

				// Add the SpriteRendererComponent
				SpriteRendererComponent comp;
				auto& spriteJson = entJson["SpriteRenderer"];
				if (spriteJson.contains("Colour"))
					comp.Colour = spriteJson["Colour"].get<glm::vec4>();

				Entity entity{ targetEntity, m_Scene };
				entity.AddComponent<SpriteRendererComponent>(comp);
			}
		}
	};
}