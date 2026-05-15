#include <bp_pch.h>
#include "Camera.h"
#include <glm/gtc/matrix_inverse.hpp>

namespace BitPounce
{
    glm::vec2 Camera::PixelToWorld(glm::ivec2 pixel, const glm::mat4& projection, const glm::ivec2 res)
    {
        float ndcX = (2.0f * pixel.x) / res.x - 1.0f;
        float ndcY = 1.0f - (2.0f * pixel.y) / res.y;

        glm::vec4 clipNear(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 clipFar(ndcX, ndcY,  1.0f, 1.0f);

        glm::mat4 invProj = glm::inverse(projection);
        glm::vec4 viewNear = invProj * clipNear;
        glm::vec4 viewFar  = invProj * clipFar;
        viewNear /= viewNear.w;
        viewFar  /= viewFar.w;

        glm::vec3 rayOrigin = glm::vec3(viewNear);
        glm::vec3 rayDir    = glm::normalize(glm::vec3(viewFar - viewNear));


        if (rayDir.z == 0.0f)
            return glm::vec2(0.0f);

        float t = -rayOrigin.z / rayDir.z;
        if (t < 0.0f) t = 0.0f;

        glm::vec3 hitPoint = rayOrigin + t * rayDir;

        return glm::vec2(hitPoint.x, hitPoint.y);
    }
}