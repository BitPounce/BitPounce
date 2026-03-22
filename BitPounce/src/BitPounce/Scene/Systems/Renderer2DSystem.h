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
		
		    // Draw Sprites
		    auto spriteView = m_Scene->GetRegistry(*this).view<TransformComponent, SpriteRendererComponent>();
		    for (auto entity : spriteView)
		    {
		        auto& transform = spriteView.get<TransformComponent>(entity);
		        auto& sprite = spriteView.get<SpriteRendererComponent>(entity);
			
		        if(sprite.Texture)
		        {
		            Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.Colour, sprite.TilingFactor, (int)entity);
		        }
		        else
		        {
		            Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entity);
		        }
		    }
		
		    // Draw Circles
		    auto circleView = m_Scene->GetRegistry(*this).view<TransformComponent, CircleRendererComponent>();
		    for (auto entity : circleView)
		    {
		        auto& transform = circleView.get<TransformComponent>(entity);
		        auto& circle = circleView.get<CircleRendererComponent>(entity);
			
		        Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entity);
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
		

		virtual void Serialize(nlohmann::json& json) override
		{
		    auto& registry = m_Scene->GetRegistry(*this);
		
		    // Serialize SpriteRendererComponent
		    auto spriteView = registry.view<SpriteRendererComponent>();
		    for (auto entity : spriteView)
		    {
		        auto& sprite = spriteView.get<SpriteRendererComponent>(entity);
			
		        nlohmann::json spriteJson;
		        spriteJson["Colour"] = sprite.Colour;
			
		        for (auto& ent : json["Entities"])
		        {
		            if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
		            {
		                ent["SpriteRenderer"] = spriteJson;
		                break;
		            }
		        }
		    }
		
		    // Serialize CircleRendererComponent
		    auto circleView = registry.view<CircleRendererComponent>();
		    for (auto entity : circleView)
		    {
		        auto& circle = circleView.get<CircleRendererComponent>(entity);
			
		        nlohmann::json circleJson;
		        circleJson["Colour"] = circle.Colour;
		        circleJson["Fade"] = circle.Fade;
		        circleJson["Thickness"] = circle.Thickness;
			
		        for (auto& ent : json["Entities"])
		        {
		            if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
		            {
		                ent["CircleRenderer"] = circleJson;
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
			
		        Entity entity{ targetEntity, m_Scene };
			
		        if (entJson.contains("SpriteRenderer"))
		        {
		            SpriteRendererComponent comp;
		            auto& spriteJson = entJson["SpriteRenderer"];
		            if (spriteJson.contains("Colour"))
		                comp.Colour = spriteJson["Colour"].get<glm::vec4>();
		            entity.AddComponent<SpriteRendererComponent>(comp);
		        }
			
		        if (entJson.contains("CircleRenderer"))
		        {
		            CircleRendererComponent comp;
		            auto& circleJson = entJson["CircleRenderer"];
		            comp.Colour = circleJson["Colour"].get<glm::vec4>();
		            comp.Fade = circleJson["Fade"].get<float>();
		            comp.Thickness = circleJson["Thickness"].get<float>();
		            entity.AddComponent<CircleRendererComponent>(comp);
		        }
		    }
		}
	};
}