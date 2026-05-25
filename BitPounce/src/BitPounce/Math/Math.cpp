#include "bp_pch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace BitPounce::Math {

	

   glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
    {
        glm::quat q =
            glm::angleAxis(rotation.z, glm::vec3(0,0,1)) *
            glm::angleAxis(rotation.y, glm::vec3(0,1,0)) *
            glm::angleAxis(rotation.x, glm::vec3(1,0,0));   

        glm::mat4 R = glm::toMat4(q);   

        return glm::translate(glm::mat4(1.0f), translation)
             * R
             * glm::scale(glm::mat4(1.0f), scale);
    }
}