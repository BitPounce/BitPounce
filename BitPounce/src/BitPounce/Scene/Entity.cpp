#include "Entity.h"

namespace BitPounce
{
    Entity::Entity(entt::entity handle, Scene *scene) : m_EntityHandle(handle), m_Scene(scene)
    {
    }

    Entity::Entity(): m_EntityHandle(entt::null), m_Scene(nullptr)
    {

    }
}