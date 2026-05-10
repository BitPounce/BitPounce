#include "Player.h"
#include "Generation.h"

static int s_IMG_X = 0;
static int s_IMG_Y = 0;

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

        // No input? skip movement
        if (dir.x == 0 && dir.y == 0)
            continue;

        //dir = glm::normalize(dir);
        const float speed = 6.25f;      // (5/2)*(5/2) – adjust as needed
        dir *= speed * ts;              // apply speed and delta time

        BitPounce::Rect<float> wallRect{};
        wallRect.h = 1;
        wallRect.w = 1;
        BitPounce::Rect<float> playerRect{};
        playerRect.x = (transform.Translation + glm::vec3(dir, 0.0f)).x;
        playerRect.y = (transform.Translation + glm::vec3(dir, 0.0f)).y;
        playerRect.h = 1;
        playerRect.w = 1;
        //for(auto&& tile : player.tilemap.GetComponent<BitPounce::TilemapComponent>().tiles)
        //{
        //    if(tile.tex != WALL_0) {continue;}
        //    wallRect.x = tile.pos.x - 0.5f;
        //    wallRect.y = tile.pos.y - 0.5f;
        //    if(BitPounce::RectCheckCollision(playerRect, wallRect))
        //    {
        //        return;
        //    }
        //}
        sr.SpriteIndex = glm::ivec2(player.frime, 15);

        transform.Translation +=  glm::vec3(dir, 0.0f) * 0.5f;
        transform.Translation +=  glm::vec3(dir, 0.0f) * 0.5f;

        // Move camera (only if this is the main player)
        auto cam = m_Scene->GetActiveCamera();
        if (cam.second)
            cam.second->Translation = transform.Translation;
        
    }
}

void PlayerSystem::OnImGuiDraw(BitPounce::Timestep &ts)
{
    ImGui::Begin("Player");
    ImGui::DragInt("X", &s_IMG_X);
    ImGui::DragInt("Y", &s_IMG_Y);
    ImGui::End();
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
