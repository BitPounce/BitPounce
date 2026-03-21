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
		Renderer2DSystem() { m_name = "Renderer 2D System"; };
		virtual System* clone() const override {
        	return new Renderer2DSystem(*this);
    	}

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

			{
				auto group = m_Scene->GetRegistry(*this).group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto entity : group)
				{
					auto&& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					if(sprite.Texture)
					{
						Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.Colour, sprite.TilingFactor, (int)entity);
					}
					else
					{
						Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entity);
					}

				}
			}

			{
				auto group = m_Scene->GetRegistry(*this).group<CircleRendererComponent>(entt::get<TransformComponent>);
				for (auto entity : group)
				{
					auto&& [transform, circle] = group.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);

				}
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

			if (ImGui::MenuItem("Circle Renderer"))
			{
				ent.AddComponent<CircleRendererComponent>();
				ImGui::CloseCurrentPopup();
			}
		};

		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			ImGuiUtils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component)
			{
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
			});

			ImGuiUtils::DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component)
			{
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
			});
		}
		

		virtual void Serialize(nlohmann::json& json)
		{
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

			{
				auto group = m_Scene->GetRegistry(*this).group<CircleRendererComponent>();
				for (auto entity : group)
				{
					
				
					auto&& circle = group.get<CircleRendererComponent>(entity);
				
					nlohmann::json circleRendererComponent = nlohmann::json();
					circleRendererComponent["Colour"] = circle.Colour;
					circleRendererComponent["Fade"] = circle.Fade;
					circleRendererComponent["Thickness"] = circle.Thickness;
				
					nlohmann::json* entjson = nullptr;
					for(auto& ent : json["Entities"])
					{
						auto entityID = ent["entityID"];
						if(entityID.get<uint32_t>() == (uint32_t)entity)
						{
							ent["CircleRenderer"] = circleRendererComponent;
							break;
						}
					}
				}
			}
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			for (auto& entJson : json["Entities"])
			{
				if (!entJson.contains("entityID"))
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

				if(entJson.contains("SpriteRenderer"))
				{
					// Add the SpriteRendererComponent
					SpriteRendererComponent comp;
					auto& spriteJson = entJson["SpriteRenderer"];
					if (spriteJson.contains("Colour"))
						comp.Colour = spriteJson["Colour"].get<glm::vec4>();

					Entity entity{ targetEntity, m_Scene };
					entity.AddComponent<SpriteRendererComponent>(comp);
				}

				if(entJson.contains("CircleRenderer"))
				{
					// Add the SpriteRendererComponent
					CircleRendererComponent comp;
					auto& spriteJson = entJson["CircleRenderer"];
					comp.Colour = spriteJson["Colour"].get<glm::vec4>();
					comp.Fade = spriteJson["Fade"].get<float>();
					comp.Thickness = spriteJson["Thickness"].get<float>();

					Entity entity{ targetEntity, m_Scene };
					entity.AddComponent<CircleRendererComponent>(comp);
				}
				
			}
		}
	};
}