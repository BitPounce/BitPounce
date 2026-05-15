#include "Player.h"
#include <random>
#include "Generation.h"
#include "Shop.h"

static int s_IMG_X = 0;
static int s_IMG_Y = 0;

static bool s_Flying = false;

BP_REGISTER_CMD("player_fly", "", [](const std::vector<std::string>& args)
{
	s_Flying = !s_Flying;
});

void PlayerSystem::OnUpdate(BitPounce::Timestep &ts) 
{
	auto& registry = m_Scene->GetRegistry(*this);
	auto view = registry.view<Player, BitPounce::TransformComponent>();

	for (auto entity : view)
	{
		BitPounce::Entity e = {entity, m_Scene};
		auto& player = view.get<Player>(entity);
		auto& transform = view.get<BitPounce::TransformComponent>(entity);
		auto&& sr = e.GetComponent<BitPounce::SpriteRendererComponent>();
		auto&& healthSys = e.GetComponent<HealthSystem<float, 0.0f>>();

		auto cam = m_Scene->GetActiveCamera();
		if (cam.second)
			cam.second->Translation = transform.Translation;

		auto gun = player.gun;
		auto& gunTransform = gun.GetComponent<BitPounce::TransformComponent>();
		auto& gunSprite = gun.GetComponent<BitPounce::SpriteRendererComponent>();
		Item item = Shop_GetItem();
		gunSprite.Texture = item.texHandle;
		auto mousePos = BitPounce::Input::GetMousePosition();
		auto wordMousePos = BitPounce::Camera::PixelToWorld(mousePos, cam.first->Camera.GetProjection() * glm::inverse(cam.second->GetTransform()), glm::ivec2(BitPounce::Application::Get().GetWindow().GetWidth(), BitPounce::Application::Get().GetWindow().GetHeight()));

		glm::vec2 gunPos = gunTransform.Translation;
		glm::vec2 mouse = wordMousePos;
		glm::vec2 gunDir = gunPos - mouse;

		glm::vec2 dirToMouse = wordMousePos - glm::vec2(transform.Translation.x, transform.Translation.y);
		float length = glm::length(dirToMouse);
		if (length > 0.0001f) {
			dirToMouse = dirToMouse / length;
		}
		else {
			dirToMouse = glm::vec2(1.0f, 0.0f);
		}


		float radius = 1.2f;
		glm::vec2 offset = dirToMouse * radius;
		gunTransform.Translation = glm::vec3(
			transform.Translation.x + offset.x,
			transform.Translation.y + offset.y,
			gunTransform.Translation.z
		);

		float angle = atan2(dirToMouse.y, dirToMouse.x);
		gunTransform.Rotation.z = angle;

		if(player.Timer.Elapsed() >= 1.0f / player.fps)
		{
			player.frime++;
			player.Timer.Reset();
		}
		if(player.frime > 3)
		{
			player.frime = 0;
		}
		float sig = (sinf(player.Timer.Elapsed() * 100) + 1) / 2;
		sr.SpriteIndex = glm::ivec2(player.frime, 14);

		glm::vec2 dir{0.f, 0.f};
		if (BitPounce::Input::IsKeyPressed(BP_KEY_W)) dir.y += 1;
		if (BitPounce::Input::IsKeyPressed(BP_KEY_S)) dir.y -= 1;
		if (BitPounce::Input::IsKeyPressed(BP_KEY_A)) dir.x -= 1;
		if (BitPounce::Input::IsKeyPressed(BP_KEY_D)) dir.x += 1;
		auto iem = Shop_GetItem();
		
		if(BitPounce::Input::IsMouseButtonPressed(0) && player.Timer2.Elapsed() >= 10.f / (float)iem.fireRate)
		{
			auto mousePos = BitPounce::Input::GetMousePosition();
			auto wordMousePos = BitPounce::Camera::PixelToWorld(mousePos, cam.first->Camera.GetProjection() * glm::inverse(cam.second->GetTransform()), glm::ivec2(BitPounce::Application::Get().GetWindow().GetWidth(), BitPounce::Application::Get().GetWindow().GetHeight()));
			auto enemyView = registry.view<Enemy, BitPounce::TransformComponent>();
			for (auto entity : enemyView)
			{
				BitPounce::Entity enemyEntity = {entity, m_Scene};
				auto& enemy = enemyView.get<Enemy>(entity);
				auto& enemyTransform = enemyView.get<BitPounce::TransformComponent>(entity);
				
				if(glm::distance(wordMousePos, glm::vec2(enemyTransform.Translation)) <= iem.radius)
				{

					static std::mt19937 gen(67);
					static std::uniform_int_distribution<int> uniform_int_distribution(0, player.Audios.size() - 1);
					
					player.Audios[uniform_int_distribution(gen)]->Play();
					healthSys.Heal(5);
					Shop_AddMoney(10);
					player.onKilledEnemy();
					
					enemyEntity.Destroy();
				}
			}
		}

		{
			auto WindowsView = registry.view<Window, BitPounce::TransformComponent>();
			for (auto entity : WindowsView)
			{
				auto& window = WindowsView.get<Window>(entity);
				auto& windowTransform = WindowsView.get<BitPounce::TransformComponent>(entity);
				windowTransform.Translation.y = window.yPos + (glm::sin(window.timer) / 2);
				window.timer += ts;

				if(BitPounce::Input::IsMouseButtonPressed(0))
				{
					auto wordMousePos = BitPounce::Camera::PixelToWorld(BitPounce::Input::GetMousePosition(), cam.first->Camera.GetProjection() * glm::inverse(cam.second->GetTransform()), glm::ivec2(BitPounce::Application::Get().GetWindow().GetWidth(), BitPounce::Application::Get().GetWindow().GetHeight()));
					if(glm::distance(wordMousePos, glm::vec2(windowTransform.Translation)) <= 0.1f)
					{
						player.onWin(window.seed);
					}
				}
				else
				{
					if(glm::distance(glm::vec2(transform.Translation), glm::vec2(windowTransform.Translation)) <= 2.0f)
					{
						player.onWin(window.seed);
					}
				}

			}
		}

		// No input? skip movement
		if (dir.x == 0 && dir.y == 0)
			continue;

		//dir = glm::normalize(dir);
		const float speed = 6.25f;      // (5/2)*(5/2) – adjust as needed
		dir *= speed * ts;              // apply speed and delta time

		BitPounce::Rect<float> wallRect{};
		wallRect.h = 1 / 1.1f;
		wallRect.w = 1 / 1.1f;
		BitPounce::Rect<float> playerRect{};
		playerRect.x = (transform.Translation + glm::vec3(dir, 0.0f)).x;
		playerRect.y = (transform.Translation + glm::vec3(dir, 0.0f)).y;
		playerRect.h = 1 / 1.9;
		playerRect.w = 1 / 1.9;
		for(auto&& tile : player.tilemap.GetComponent<BitPounce::TilemapComponent>().tiles)
		{
			if(tile.tex != WALL_0) {continue;}
			wallRect.x = tile.pos.x - .5f;
			wallRect.y = tile.pos.y - .5f;
			if(!s_Flying && BitPounce::RectCheckCollision(playerRect, wallRect))
			{
				return;
			}
		}
		sr.SpriteIndex = glm::ivec2(player.frime, 15);

		transform.Translation +=  glm::vec3(dir, 0.0f) * 0.5f;
		transform.Translation +=  glm::vec3(dir, 0.0f) * 0.5f;
		player.HasMoved = true;

		// Move camera (only if this is the main player)
		if (cam.second)
			cam.second->Translation = transform.Translation;
		
	}
}

void PlayerSystem::OnImGuiDraw(BitPounce::Timestep &ts)
{
}

void PlayerSystem::OnRuntimeStart()
{
	auto& registry = m_Scene->GetRegistry(*this);
	auto view = registry.view<Player, BitPounce::TransformComponent>();

	for (auto entity : view)
	{
		auto& player = view.get<Player>(entity);
		auto& transform = view.get<BitPounce::TransformComponent>(entity);

		BitPounce::Entity ent = m_Scene->CreateEntity();
		ent.AddComponent<BitPounce::SpriteRendererComponent>();
		player.gun = ent;
	}
}

PlayerSystem::PlayerSystem() 
{

}

PlayerSystem::~PlayerSystem() 
{

}

template<>
void BitPounce::ECSSystem::OnComponentAdded<Player>(Entity entity, Player& component)
{

}

template<>
void BitPounce::ECSSystem::OnComponentAdded<::Window>(Entity entity, ::Window& component)
{
		
}