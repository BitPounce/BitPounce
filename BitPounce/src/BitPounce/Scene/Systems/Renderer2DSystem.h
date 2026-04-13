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
#include "BitPounce/Asset/AssetManager.h"
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
		float timer = 0;

		virtual void OnDraw(Timestep& ts) override 
		{
			auto campar = m_Scene->GetActiveCamera();
			if(!campar.first) { return; }
			auto mainCamera = campar.first->Camera;
			auto cameraTransform = campar.second->GetTransform();
			timer += ts;

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
			
		        if(sprite.Texture && sprite.UseSpriteSheet)
		        {
					Renderer2D::DrawQuad(transform.GetTransform(), AssetManager::GetAsset<Texture2D>(sprite.Texture), sprite.SpriteSize, sprite.SpriteIndex, sprite.Colour, (int)entity);
		        }
				else if(sprite.Texture)
				{
					Renderer2D::DrawQuad(transform.GetTransform(), AssetManager::GetAsset<Texture2D>(sprite.Texture), sprite.Colour, sprite.TilingFactor, (int)entity);
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

				if(!text.FontHandle)
				{
					continue;
				}
			
		        Renderer2D::DrawString(text.TextString, AssetManager::GetAsset<Font>(text.FontHandle), transform.GetTransform(), text.textParams, (int)entity);
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
				ImGui::Checkbox("Use Sprite Sheet", &component.UseSpriteSheet);

				if(component.Texture && component.UseSpriteSheet)
				{
					ImGui::DragInt2("Sprite Size", glm::value_ptr(component.SpriteSize), 1, 1);
					ImGui::DragInt2("Sprite Index", glm::value_ptr(component.SpriteIndex), 1, 0);
					Ref<Texture2D> Texture = AssetManager::GetAsset<Texture2D>(component.Texture);
					ImGui::ImageButton((std::string("TEX_IMG_SpriteRendererComponent") + std::to_string(entity.operator unsigned int())).c_str(), (ImTextureID)(void*)Texture->GetRendererID(), ImVec2(100.0f, 100.0f));
				}
				else if(component.Texture)
				{
					Ref<Texture2D> Texture = AssetManager::GetAsset<Texture2D>(component.Texture);
					ImGui::ImageButton((std::string("TEX_IMG_SpriteRendererComponent") + std::to_string(entity.operator unsigned int())).c_str(), (ImTextureID)(void*)Texture->GetRendererID(), ImVec2(100.0f, 100.0f));
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
						component.Texture = Project::GetActive()->GetEditorAssetManager()->ImportAsset(texturePath);
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

				if(component.FontHandle)
				{
					Ref<Texture2D> Texture = AssetManager::GetAsset<Font>(component.FontHandle)->GetAtlasTexture();
					ImGui::ImageButton((std::string("TEX_IMG_TextComponent") + std::to_string(entity.operator unsigned int())).c_str(), (ImTextureID)(void*)Texture->GetRendererID(), ImVec2(100.0f, 100.0f));
				}
				else
				{
					ImGui::Button("Font", ImVec2(100.0f, 0.0f));
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						// Making sure that the data is good is for chickens, we are not chickens!!! 🐔🐔🐔
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath(path);

						// HACK: if you do not like this you can make your own Renderer2DSystem idc.
						component.FontHandle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(texturePath);
					}
					ImGui::EndDragDropTarget();
				}
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
				spriteJson["SpriteSize"] = sprite.SpriteSize;
				spriteJson["SpriteIndex"] = sprite.SpriteIndex;
				spriteJson["UseSpriteSheet"] = sprite.UseSpriteSheet;

				if (sprite.Texture)
				{
					//std::string dgdgfdgf = std::filesystem::relative(sprite.Texture->GetPath(), Project::GetAssetDirectory()).generic_string();
					spriteJson["TextureID"] = sprite.Texture.operator std::size_t();
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

				if (text.FontHandle)
				{
					//std::string dgdgfdgf = std::filesystem::relative(sprite.Texture->GetPath(), Project::GetAssetDirectory()).generic_string();
					textJson["FontID"] = text.FontHandle.operator std::size_t();
				}
		        
			
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
					if (spriteJson.contains("TextureID"))
					{
						//std::string texturePath = spriteJson["TexturePath"].get<std::string>();
						//auto path = Project::GetAssetFileSystemPath(texturePath);
						//comp.Texture = Texture2D::Create(path.string());
						comp.Texture = spriteJson["TextureID"].get<size_t>();
					}
					if (spriteJson.contains("SpriteSize"))
						comp.SpriteSize = spriteJson["SpriteSize"].get<glm::u32vec2>();

					if (spriteJson.contains("SpriteIndex"))
						comp.SpriteIndex = spriteJson["SpriteIndex"].get<glm::u32vec2>();

					if (spriteJson.contains("UseSpriteSheet"))
						comp.UseSpriteSheet = spriteJson["UseSpriteSheet"].get<bool>();

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

					if (textJson.contains("FontID"))
					{
						comp.FontHandle = textJson["FontID"].get<size_t>();
					}

					entity.AddComponent<TextComponent>(comp);
				}
		    }
		}
	};
}