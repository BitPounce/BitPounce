#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <entt/entt.hpp>
#include <BitPounce/Renderer/Renderer2D.h>
#include "../Components.h"
#include "imgui.h"

namespace BitPounce
{
	class CameraSystem : public ECSSystem
	{
	public:
		CameraSystem() {};
		CameraSystem* clone() const override {
        	return new CameraSystem(*this);
    	}

		virtual void AddComponentPopupImguiDraw(Entity& ent) override
		{
			if (ImGui::MenuItem("Camera"))
			{
				ent.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}
		};

        virtual void OnEditorPropImguiDraw(Entity& entity)
        {
			ImGuiUtils::DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& cameraComponent)
			{
		    	auto& camera = cameraComponent.Camera;
		    	ImGui::Checkbox("Primary", &cameraComponent.Primary);
		    	const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
		    	const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
		    	if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
		    	{
		    		for (int i = 0; i < 2; i++)
		    		{
		    			bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
		    			if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
		    			{
		    				currentProjectionTypeString = projectionTypeStrings[i];
		    				camera.SetProjectionType((SceneCamera::ProjectionType)i);
		    			}
		    			if (isSelected)
		    				ImGui::SetItemDefaultFocus();
		    		}
		    		ImGui::EndCombo();
		    	}
		    	if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
		    	{
		    		float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
		    		if (ImGui::DragFloat("Vertical FOV", &verticalFov))
		    			camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
		    		float orthoNear = camera.GetPerspectiveNearClip();
		    		if (ImGui::DragFloat("Near", &orthoNear))
		    			camera.SetPerspectiveNearClip(orthoNear);
		    		float orthoFar = camera.GetPerspectiveFarClip();
		    		if (ImGui::DragFloat("Far", &orthoFar))
		    			camera.SetPerspectiveFarClip(orthoFar);
		    	}
		    	if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
		    	{
		    		float orthoSize = camera.GetOrthographicSize();
		    		if (ImGui::DragFloat("Size", &orthoSize))
		    			camera.SetOrthographicSize(orthoSize);
		    		float orthoNear = camera.GetOrthographicNearClip();
		    		if (ImGui::DragFloat("Near", &orthoNear))
		    			camera.SetOrthographicNearClip(orthoNear);
		    		float orthoFar = camera.GetOrthographicFarClip();
		    		if (ImGui::DragFloat("Far", &orthoFar))
		    			camera.SetOrthographicFarClip(orthoFar);
		    		ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.FixedAspectRatio);
		    	}
			});
        }

		virtual void Serialize(nlohmann::json& json)
		{
			auto group = m_Scene->GetRegistry(*this).group<CameraComponent>();
			for (auto entity : group)
			{
				

				auto&& camComponent = group.get<CameraComponent>(entity);

				nlohmann::json cameraComponent = nlohmann::json();
				cameraComponent["FixedAspectRatio"] = camComponent.FixedAspectRatio;
				cameraComponent["Primary"] = camComponent.Primary;
				nlohmann::json camera = nlohmann::json();

				auto cam = camComponent.Camera;
				camera["AspectRatio"] = cam.GetAspectRatio();
				camera["ProjectionType"] = cam.GetProjectionType();
				
				nlohmann::json perspective = nlohmann::json();
				perspective["POV"] = cam.GetPerspectiveVerticalFOV();
				perspective["NearClip"] = cam.GetPerspectiveNearClip();
				perspective["FarClip"] = cam.GetPerspectiveFarClip();

				camera["Perspective"] = perspective;

				nlohmann::json orthographic = nlohmann::json();
				orthographic["Size"] = cam.GetOrthographicSize();
				orthographic["NearClip"] = cam.GetOrthographicNearClip();
				orthographic["FarClip"] = cam.GetOrthographicFarClip();

				camera["Orthographic"] = orthographic;

				cameraComponent["Camera"] = camera;

				nlohmann::json* entjson = nullptr;
				for(auto& ent : json["Entities"])
				{
					auto entityID = ent["entityID"];
					if(entityID.get<uint32_t>() == (uint32_t)entity)
					{
						ent["CameraComponent"] = cameraComponent;
					}
				}
			}
			
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			for (auto& entJson : json["Entities"])
			{
				if (!entJson.contains("entityID") || !entJson.contains("CameraComponent"))
					continue;

				uint32_t id = entJson["entityID"].get<uint32_t>();

				// Find entity by matching entityID
				entt::entity targetEntity = entt::null;
				auto view = registry.view<TagComponent>(); // use any guaranteed component
				for (auto e : view)
				{
					Entity entity{ e, m_Scene };
					if ((uint32_t)entity == id)
					{
						targetEntity = e;
						break;
					}
				}

				if (targetEntity == entt::null) continue;

				auto& camJson = entJson["CameraComponent"];
				CameraComponent comp;

				if (camJson.contains("Primary")) comp.Primary = camJson["Primary"].get<bool>();
				if (camJson.contains("FixedAspectRatio")) comp.FixedAspectRatio = camJson["FixedAspectRatio"].get<bool>();

				if (camJson.contains("Camera"))
				{
					auto& cameraJson = camJson["Camera"];
					auto& cam = comp.Camera;

					if (cameraJson.contains("ProjectionType")) cam.SetProjectionType((SceneCamera::ProjectionType)cameraJson["ProjectionType"].get<int>());

					if (cameraJson.contains("Perspective"))
					{
						auto& pJson = cameraJson["Perspective"];
						if (pJson.contains("POV")) cam.SetPerspectiveVerticalFOV(pJson["POV"].get<float>());
						if (pJson.contains("NearClip")) cam.SetPerspectiveNearClip(pJson["NearClip"].get<float>());
						if (pJson.contains("FarClip")) cam.SetPerspectiveFarClip(pJson["FarClip"].get<float>());
					}

					if (cameraJson.contains("Orthographic"))
					{
						auto& oJson = cameraJson["Orthographic"];
						if (oJson.contains("Size")) cam.SetOrthographicSize(oJson["Size"].get<float>());
						if (oJson.contains("NearClip")) cam.SetOrthographicNearClip(oJson["NearClip"].get<float>());
						if (oJson.contains("FarClip")) cam.SetOrthographicFarClip(oJson["FarClip"].get<float>());
					}
				}

				Entity entity{ targetEntity, m_Scene };
				entity.AddComponent<CameraComponent>(comp);
			}
		}
	};

	
}