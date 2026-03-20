#include <bp_pch.h>
#include "Components.h"
#include "Entity.h"

namespace BitPounce
{
    Entity::Entity(entt::entity handle, Scene *scene) : m_EntityHandle(handle), m_Scene(scene)
    {
    }

    Entity::Entity(): m_EntityHandle(entt::null), m_Scene(nullptr)
    {

    }
    TransformComponent& Entity::GetTransform()
    {
        return GetComponent<TransformComponent>();
    }
    std::vector<Entity>& Entity::GetChildren()
    {
        return GetComponent<ChildrenComponent>().children;
    }
    Entity& Entity::GetParent()
    {
        return GetComponent<TransformComponent>().Parent;
    }

    void Entity::SetParent(Entity &ent)
    {
        GetTransform().Parent = ent;
        ent.GetChildren().emplace_back(*this);

    }
    void Entity::OnDestroy()
    {
        Entity parent = GetParent();
        if(parent)
        {
            parent.GetChildren().erase(std::remove(std::begin(parent.GetChildren()), std::end(parent.GetChildren()), *this), std::end(parent.GetChildren()));
        }
    }
}