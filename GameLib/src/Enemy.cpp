#include "Enemy.h"

void EnemySystem::OnUpdate(BitPounce::Timestep &ts)
{
	auto& registry = m_Scene->GetRegistry(*this);
	{
		auto view = registry.view<Enemy, BitPounce::TransformComponent>();

		for (auto entity : view)
		{
			BitPounce::Entity e = { entity, m_Scene };
			auto& enemy = view.get<Enemy>(entity);
			if (!enemy.Player.GetComponent<Player>().HasMoved) { return; }
			auto& transform = view.get<BitPounce::TransformComponent>(entity);
			auto&& sr = e.GetComponent<BitPounce::SpriteRendererComponent>();

			if (enemy.Timer.Elapsed() >= 1.0f / enemy.fps)
			{
				enemy.frime++;
				enemy.Timer.Reset();
			}
			if (enemy.frime > 3)
			{
				enemy.frime = 0;
			}

			sr.SpriteIndex = glm::ivec2(enemy.frime, 2);

			if (glm::distance(e.GetTransform().Translation, enemy.Player.GetTransform().Translation) <= .5f)
			{
				if (enemy.Timer2.Elapsed() >= .5f)
				{
					auto&& h = enemy.Player.GetComponent<HealthSystem<float, 0.0f>>();
					float damage = 10.0f;
					if (e.HasComponent<SwapEnemy>())
						damage *= 2;
					h.Damage(damage);
					enemy.Timer2.Reset();
				}

			}

			if (glm::distance(e.GetTransform().Translation, enemy.Player.GetTransform().Translation) <= 7.0f)
			{
				sr.SpriteIndex = glm::ivec2(enemy.frime, 3);
				e.GetTransform().Translation += glm::normalize(glm::vec3(enemy.Player.GetTransform().Translation - e.GetTransform().Translation)) * enemy.speed * ts.GetSeconds();
			}
		}
	}
	{
		auto view = registry.view<Enemy, SwapEnemy>();

		for (auto entity : view)
		{
			BitPounce::Entity e = { entity, m_Scene };
			auto& enemy = view.get<Enemy>(entity);
			if (!enemy.Player.GetComponent<Player>().HasMoved) { return; }
			auto& swap = view.get<SwapEnemy>(entity);
			auto& transform = e.GetComponent<BitPounce::TransformComponent>();

			if (swap.Timer.Elapsed() >= swap.swapCallback)
			{
				float MaxDis = glm::distance(transform.Translation, enemy.Player.GetTransform().Translation);
				auto EnemyView = registry.view<Enemy, BitPounce::TransformComponent>();
				BitPounce::Entity ent{};
				for (auto entity2 : EnemyView)
				{
					BitPounce::Entity e2 = { entity2, m_Scene };
					if (entity == entity2)
						continue;
					auto& t2 = e2.GetTransform();
					if (glm::distance(transform.Translation, t2.Translation) < MaxDis)
					{
						MaxDis = glm::distance(transform.Translation, t2.Translation);
						ent = e2;
					}
				}
				if (!ent) { continue; }
				auto oldPos = transform.Translation;
				transform.Translation = ent.GetTransform().Translation;
				ent.GetTransform().Translation = oldPos;
			}
			
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

	template<>
	void ECSSystem::OnComponentAdded<SwapEnemy>(Entity entity, SwapEnemy& component)
	{
	}
}