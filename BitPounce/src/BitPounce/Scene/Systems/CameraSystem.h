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

        virtual void OnEditorPropImguiDraw(Entity& entity)
        {
            if (entity.HasComponent<CameraComponent>())
		    {
		    	if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
		    	{
		    		auto& cameraComponent = entity.GetComponent<CameraComponent>();
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
                    ImGui::TreePop();
		    	}
            }
        }
	};
}