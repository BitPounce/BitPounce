#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include <BitPounce/Renderer/Renderer2D.h>
#include "../Components.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "BitPounce/ImGui/ImGuiUtils.h"
#include "BitPounce/Project/Project.h"
#include <misc/cpp/imgui_stdlib.h>

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

			auto textView = m_Scene->GetRegistry(*this).view<TransformComponent, TextComponent>();
		    for (auto entity : textView)
		    {
		        auto& transform = textView.get<TransformComponent>(entity);
		        auto& text = textView.get<TextComponent>(entity);
			
		        Renderer2D::DrawString(text.TextString, text.FontAsset, transform.GetTransform(), text.textParams, (int)entity);
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

			if (ImGui::MenuItem("Text Component"))
			{
				auto&& text = ent.AddComponent<TextComponent>();
				text.TextString = "Hello World!";
				ImGui::CloseCurrentPopup();
			}
		};

		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			ImGuiUtils::DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [entity](SpriteRendererComponent& component)
			{
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));

				if(component.Texture)
				{
					ImGui::ImageButton((std::string("TEX_IMG_SpriteRendererComponent") + std::to_string(entity.operator unsigned int())).c_str(), (ImTextureID)(void*)component.Texture->GetRendererID(), ImVec2(100.0f, 100.0f));
				}
				else
				{
					ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						// Making sure that the data is good is for chickens, we are not chickens!!! 🐔🐔🐔
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath(path);

						// HACK: if you do not like this you can make your own Renderer2DSystem idc.
						texturePath = std::filesystem::relative(texturePath, Project::GetAssetDirectory());
						Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
						component.Texture = texture;
					}
					ImGui::EndDragDropTarget();
				}
			});

			ImGuiUtils::DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component)
			{
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
				ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
				ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);

				
			});

			ImGuiUtils::DrawComponent<TextComponent>("Text Component", entity, [entity](TextComponent& component)
			{
				ImGui::InputTextMultiline("Text String", &component.TextString);
				ImGui::ColorEdit4("Colour", glm::value_ptr(component.textParams.Colour));
				ImGui::DragFloat("Kerning", &component.textParams.Kerning, 0.025f);
				ImGui::DragFloat("Line Spacing", &component.textParams.LineSpacing, 0.025f);
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
				if (sprite.Texture)
				{
					std::string dgdgfdgf = std::filesystem::relative(sprite.Texture->GetPath(), Project::GetAssetDirectory()).generic_string();
					spriteJson["TexturePath"] =  dgdgfdgf;
				}
					
			
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

			auto textView = registry.view<TextComponent>();
		    for (auto entity : textView)
		    {
		        auto& text = textView.get<TextComponent>(entity);
			
		        nlohmann::json textJson;
		        textJson["TextString"] = text.TextString;
		        textJson["Colour"] = text.textParams.Colour;
				textJson["Kerning"] = text.textParams.Kerning;
				textJson["LineSpacing"] = text.textParams.LineSpacing;
		        
			
		        for (auto& ent : json["Entities"])
		        {
		            if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
		            {
		                ent["TextComponent"] = textJson;
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
					if (spriteJson.contains("TexturePath"))
					{
						std::string texturePath = spriteJson["TexturePath"].get<std::string>();
						auto path = Project::GetAssetFileSystemPath(texturePath);
						comp.Texture = Texture2D::Create(path.string());
					}
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

				if(entJson.contains("TextComponent"))
				{
					TextComponent comp;
					auto& textJson = entJson["TextComponent"];
					comp.TextString = textJson["TextString"].get<std::string>();
					comp.textParams.Colour = textJson["Colour"].get<glm::vec4>();
					comp.textParams.Kerning = textJson["Kerning"].get<float>();
					comp.textParams.LineSpacing = textJson["LineSpacing"].get<float>();
					entity.AddComponent<TextComponent>(comp);
				}
		    }
		}
	};
}