#pragma once

#include "Model.h"
#include "Material.h"

namespace BitPounce
{
    class Renderer3D
    {
    public:

        static void Init();
	    static void Shutdown();

        struct Renderer3DSceneData
        {
            // TODO: more Lights
            glm::vec3 mainLightPos = glm::vec3(0, 10, 0);
            glm::vec4 mainLightColour = glm::vec4(1,1,1,1);
            glm::vec3 camPos;
        };

        static void BeginScene(const glm::mat4& matrix, const Renderer3DSceneData& sceneData);
        static void EndScene();

        static void DrawModel(const glm::mat4& tr, const Ref<Model> model, Ref<Material> shader);
        static void DrawModel(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3 &scale, const Ref<Model> model, Ref<Material> shader);
        
    };
}