#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include <BitPounce/Renderer/Renderer2D.h>
#include "../Components.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <functional>

#include "BitPounce/ImGui/ImGuiUtils.h"
#include "BitPounce/Project/Project.h"
#include "BitPounce/Asset/AssetManager.h"

namespace BitPounce
{
	class Renderer2DSystem : public ECSSystem
	{
	public:
		Renderer2DSystem()
		{
			m_name = "Renderer 2D System";
		}

		virtual System* clone() const override
		{
			return new Renderer2DSystem(*this);
		}

		float timer = 0.0f;

		void DrawTilemaps()
		{
			auto view = m_Scene->GetRegistry(*this).view<TilemapComponent>();

			for (auto entity : view)
			{
				auto& tilemap = view.get<TilemapComponent>(entity);

				// Render the greedy mesh (each GreedyQuad becomes one draw call)
				for (const auto& quad : tilemap.renderer2D_tiles)
				{
				    Renderer2D::DrawQuad(quad.pos, AssetManager::GetAsset<Texture2D>(quad.Texture));
				}
			}
		}

		virtual void Serialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			auto spriteView = registry.view<SpriteRendererComponent>();
			for (auto entity : spriteView)
			{
				auto& sprite = spriteView.get<SpriteRendererComponent>(entity);

				nlohmann::json j;
				j["Colour"] = sprite.Colour;
				j["SpriteSize"] = sprite.SpriteSize;
				j["SpriteIndex"] = sprite.SpriteIndex;
				j["UseSpriteSheet"] = sprite.UseSpriteSheet;

				if (sprite.Texture)
					j["TextureID"] = sprite.Texture.operator uint64_t();

				for (auto& ent : json["Entities"])
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["SpriteRenderer"] = j;
			}

			auto circleView = registry.view<CircleRendererComponent>();
			for (auto entity : circleView)
			{
				auto& circle = circleView.get<CircleRendererComponent>(entity);

				nlohmann::json j;
				j["Colour"] = circle.Colour;
				j["Fade"] = circle.Fade;
				j["Thickness"] = circle.Thickness;

				for (auto& ent : json["Entities"])
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["CircleRenderer"] = j;
			}

			auto textView = registry.view<TextComponent>();
			for (auto entity : textView)
			{
				auto& text = textView.get<TextComponent>(entity);

				nlohmann::json j;
				j["TextString"] = text.TextString;
				j["Colour"] = text.textParams.Colour;
				j["Kerning"] = text.textParams.Kerning;
				j["LineSpacing"] = text.textParams.LineSpacing;

				if (text.FontHandle)
					j["FontID"] = text.FontHandle.operator uint64_t();

				for (auto& ent : json["Entities"])
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["TextComponent"] = j;
			}

			auto tilemapView = registry.view<TilemapComponent>();
			for (auto entity : tilemapView)
			{
				auto& tilemap = tilemapView.get<TilemapComponent>(entity);

				nlohmann::json j;
				j["Tiles"] = nlohmann::json::array();


				for (auto& ent : json["Entities"])
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["TilemapComponent"] = j;
			}
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			for (auto& entJson : json["Entities"])
			{
				if (!entJson.contains("entityID"))
					continue;

				uint32_t id = entJson["entityID"];
				entt::entity target = entt::null;

				auto view = registry.view<TagComponent>();
				for (auto e : view)
				{
					Entity ent{ e, m_Scene };
					if ((uint32_t)ent == id)
					{
						target = e;
						break;
					}
				}

				if (target == entt::null)
					continue;

				Entity entity{ target, m_Scene };

				if (entJson.contains("SpriteRenderer"))
				{
					auto& j = entJson["SpriteRenderer"];
					SpriteRendererComponent c;

					c.Colour = j["Colour"].get<glm::vec4>();
					if (j.contains("SpriteSize"))
						c.SpriteSize = j["SpriteSize"].get<glm::ivec2>();
					if (j.contains("SpriteIndex"))
						c.SpriteIndex = j["SpriteIndex"].get<glm::ivec2>();
					if (j.contains("UseSpriteSheet"))
						c.UseSpriteSheet = j["UseSpriteSheet"];

					//if (j.contains("TextureID"))
					//	c.Texture = j["TextureID"].get<uint64_t>();

					entity.AddComponent<SpriteRendererComponent>(c);
				}

				if (entJson.contains("CircleRenderer"))
				{
					auto& j = entJson["CircleRenderer"];
					CircleRendererComponent c;

					c.Colour = j["Colour"];
					c.Fade = j["Fade"];
					c.Thickness = j["Thickness"];

					entity.AddComponent<CircleRendererComponent>(c);
				}

				if (entJson.contains("TextComponent"))
				{
					auto& j = entJson["TextComponent"];
					TextComponent c;

					c.TextString = j["TextString"];
					c.textParams.Colour = j["Colour"];
					c.textParams.Kerning = j["Kerning"];
					c.textParams.LineSpacing = j["LineSpacing"];

					//if (j.contains("FontID"))
					//	c.FontHandle = j["FontID"].get<uint64_t>();

					entity.AddComponent<TextComponent>(c);
				}

				//if (entJson.contains("TilemapComponent"))
				//{
				//	auto& j = entJson["TilemapComponent"];
				//	TilemapComponent c;
//
				//	if (j.contains("Tiles"))
				//	{
				//		for (auto& t : j["Tiles"])
				//		{
				//			Tile tile;
				//			tile.pos = t["Pos"];
				//			tile.Texture = t["Texture"];
				//			c.renderer2D_tiles.push_back(tile);
				//		}
				//	}
//
				//	entity.AddComponent<TilemapComponent>(c);
				//}
			}
		}

		virtual void OnDraw(Timestep& ts) override
		{
			auto cam = m_Scene->GetActiveCamera();
			if (!cam.first) return;

			timer += ts;

			Draw(cam.first->Camera.GetProjection() * glm::inverse(cam.second->GetTransform()));
		}

		virtual void OnDrawEditor(Timestep& ts, EditorCamera& cam) override
		{
			Draw(cam.GetViewProjection());
		}

		void Draw(const glm::mat4& viewProj)
		{
			Renderer2D::BeginScene(viewProj);


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

			auto circleView = m_Scene->GetRegistry(*this)
				.view<TransformComponent, CircleRendererComponent>();

			for (auto e : circleView)
			{
				auto& t = circleView.get<TransformComponent>(e);
				auto& c = circleView.get<CircleRendererComponent>(e);

				Renderer2D::DrawCircle(t.GetTransform(), c.Colour, c.Thickness, c.Fade, (int)e);
			}

			auto textView = m_Scene->GetRegistry(*this).view<TransformComponent, TextComponent>();

			for (auto e : textView)
			{
				auto& t = textView.get<TransformComponent>(e);
				auto& txt = textView.get<TextComponent>(e);

				if (!txt.FontHandle)
					continue;

				Renderer2D::DrawString(txt.TextString, AssetManager::GetAsset<Font>(txt.FontHandle), t.GetTransform(), txt.textParams, (int)e);
			}

			DrawTilemaps();

			Renderer2D::EndScene();
		}

		virtual void AddComponentPopupImguiDraw(Entity& ent) override
		{
			if (ImGui::MenuItem("Sprite Renderer"))
				ent.AddComponent<SpriteRendererComponent>();

			if (ImGui::MenuItem("Circle Renderer"))
				ent.AddComponent<CircleRendererComponent>();

			if (ImGui::MenuItem("Text Component"))
				ent.AddComponent<TextComponent>();

			if (ImGui::MenuItem("Tilemap"))
				ent.AddComponent<TilemapComponent>();

			ImGui::CloseCurrentPopup();
			}
			
		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			ImGuiUtils::DrawComponent<SpriteRendererComponent>( "Sprite Renderer", entity,
				[](SpriteRendererComponent& component)
				{
					ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
					ImGui::Checkbox("Use Sprite Sheet", &component.UseSpriteSheet);
				
					if (component.Texture && component.UseSpriteSheet)
					{
						ImGui::DragInt2("Sprite Size", glm::value_ptr(component.SpriteSize), 1, 1);
						ImGui::DragInt2("Sprite Index", glm::value_ptr(component.SpriteIndex), 1, 0);
					
						Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(component.Texture);
						ImGui::ImageButton("Texture", (ImTextureID)(void*)texture->GetRendererID(), ImVec2(100.0f, 100.0f));
					}
					else if (component.Texture)
					{
						Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(component.Texture);
						ImGui::ImageButton("Texture", (ImTextureID)(void*)texture->GetRendererID(), ImVec2(100.0f, 100.0f));
					}
					else
					{
						ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
					}
				
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path texturePath(path);
						
							component.Texture = Project::GetActive()->GetEditorAssetManager()->ImportAsset(texturePath);
						}
						ImGui::EndDragDropTarget();
					}
				}
			);
		
			ImGuiUtils::DrawComponent<CircleRendererComponent>("Circle Renderer", entity,
				[](CircleRendererComponent& component)
				{
					ImGui::ColorEdit4("Colour", glm::value_ptr(component.Colour));
					ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
					ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
				}
			);
		
			ImGuiUtils::DrawComponent<TextComponent>("Text Component", entity,
				[](TextComponent& component)
				{
					ImGui::InputTextMultiline("Text String", &component.TextString);
					ImGui::ColorEdit4("Colour", glm::value_ptr(component.textParams.Colour));
					ImGui::DragFloat("Kerning", &component.textParams.Kerning, 0.025f);
					ImGui::DragFloat("Line Spacing", &component.textParams.LineSpacing, 0.025f);
				
					if (component.FontHandle)
					{
						Ref<Texture2D> texture =
							AssetManager::GetAsset<Font>(component.FontHandle)->GetAtlasTexture();
					
						ImGui::ImageButton("Font", (ImTextureID)(void*)texture->GetRendererID(), ImVec2(100.0f, 100.0f));
					}
					else
					{
						ImGui::Button("Font", ImVec2(100.0f, 0.0f));
					}
				
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path fontPath(path);
						
							component.FontHandle =
								Project::GetActive()->GetEditorAssetManager()->ImportAsset(fontPath);
						}
						ImGui::EndDragDropTarget();
					}
				}
			);
		
			//ImGuiUtils::DrawComponent<TilemapComponent>(
			//	"Tilemap",
			//	entity,
			//	[](TilemapComponent& component)
			//	{
			//		ImGui::Text("Tile Count: %zu", component.renderer2D_tiles.size());
		//
			//		if (ImGui::Button("Clear Tiles"))
			//			component.renderer2D_tiles.clear();
		//
			//		ImGui::Separator();
		//
			//		for (size_t i = 0; i < component.renderer2D_tiles.size(); i++)
			//		{
			//			auto& tile = component.renderer2D_tiles[i];
		//
			//			ImGui::PushID((int)i);
		//
			//			ImGui::DragFloat3("Position", glm::value_ptr(tile.pos), 0.1f);
			//			ImGui::InputScalar("Texture", ImGuiDataType_U64, &tile.Texture);
		//
			//			if (ImGui::Button("Remove Tile"))
			//			{
			//				component.renderer2D_tiles.erase(component.renderer2D_tiles.begin() + i);
			//				ImGui::PopID();
			//				break;
			//			}
		//
			//			ImGui::Separator();
		//
			//			ImGui::PopID();
			//		}
		//
			//		if (ImGui::Button("Add Tile"))
			//		{
			//			Tile tile{};
			//			tile.pos = glm::vec3(0.0f);
			//			tile.Texture = 0;
			//			component.renderer2D_tiles.push_back(tile);
			//		}
			//	}
			;
		}

	private:
		std::vector<std::function<void()>> m_Callbacks;
	};
}