#include "Enemy.h"

void EnemySystem::OnUpdate(BitPounce::Timestep &ts)
{
    auto& registry = m_Scene->GetRegistry(*this);
    auto view = registry.view<Enemy, BitPounce::TransformComponent>();

    for (auto entity : view)
    {
        BitPounce::Entity e = {entity, m_Scene};
        auto& enemy = view.get<Enemy>(entity);
        if(!enemy.Player.GetComponent<Player>().HasMoved) { return; }
        auto& transform = view.get<BitPounce::TransformComponent>(entity);
        auto&& sr = e.GetComponent<BitPounce::SpriteRendererComponent>();

        if(enemy.Timer.Elapsed() >= 1.0f / enemy.fps)
        {
            enemy.frime++;
            enemy.Timer.Reset();
        }
        if(enemy.frime > 3)
        {
            enemy.frime = 0;
        }

        sr.SpriteIndex = glm::ivec2(enemy.frime, 2);

        if(glm::distance(e.GetTransform().Translation, enemy.Player.GetTransform().Translation) <= .5f)
        {
            if(enemy.Timer2.Elapsed() >= .5f)
            {
                auto&& h = enemy.Player.GetComponent<HealthSystem<float, 0.0f>>();
                h.Damage(10);
                enemy.Timer2.Reset();
            }

            return;
        }

        if(glm::distance(e.GetTransform().Translation, enemy.Player.GetTransform().Translation) <= 7.0f)
        {
            sr.SpriteIndex = glm::ivec2(enemy.frime, 3);
            e.GetTransform().Translation += glm::normalize(glm::vec3(enemy.Player.GetTransform().Translation - e.GetTransform().Translation)) * enemy.speed * ts.GetSeconds();
        }
    }
}

EnemySystem::EnemySystem() {}

EnemySystem::~EnemySystem() {}


namespace BitPounce
{
    template<>
    void ECSSystem::OnComponentAdded<HealthSystem<float, 0.0f>>(Entity entity, HealthSystem<float, 0.0f>& component)
    {
    }

    template<>
    void ECSSystem::OnComponentAdded<Enemy>(Entity entity, Enemy& component)
    {
    }
}