#include <BitPounce.h>
#include "Player.h"
#include "Enemy.h"
#include "Generation.h"



struct GameCallbacks
{
    std::function<void()> PlayerDied;
    std::function<void(uint32_t windowSeed, std::mt19937& rng)> InAWindow;
    std::function<void()> OnKilledEnemy;
};

BitPounce::Ref<BitPounce::Scene> GameLoad(GameCallbacks& gameCallbacks, uint32_t seed)
{
    BitPounce::Ref<BitPounce::Scene> scene = BitPounce::CreateRef<BitPounce::Scene>();
    scene->AddSystem<BitPounce::Renderer2DSystem>();
    scene->AddSystem<BitPounce::CameraSystem>();
    // Who needs a Physics System? Not ME!!!!
    //scene->AddSystem<BitPounce::Physics2DSystem>();
    scene->AddSystem<BitPounce::AngelScriptSystem>();
    scene->AddSystem<PlayerSystem>();
    scene->AddSystem<EnemySystem>();


    BitPounce::Entity camEnt = scene->CreateEntity("Camera");
    BitPounce::CameraComponent& cam = camEnt.AddComponent<BitPounce::CameraComponent>();
    cam.Camera.SetOrthographicSize(10);
    BitPounce::Entity player = scene->CreateEntity("Player");
    auto&& playerRendor = player.AddComponent<BitPounce::SpriteRendererComponent>();
    auto&& playerRB = player.AddComponent<BitPounce::Rigidbody2DComponent>();
    auto&& playerBox = player.AddComponent<BitPounce::BoxCollider2DComponent>();
    auto&& healthSys = player.AddComponent<HealthSystem<float, 0.0f>>(100.0f);
    healthSys.OnDied += [gameCallbacks](float newHealth,float MaxHealth,float oldHealth, HealthSystem<float, 0.0f> healthSystem){
        gameCallbacks.PlayerDied();
    };
    auto&& playerTransform = player.GetComponent<BitPounce::TransformComponent>();
    playerRB.Type = BitPounce::Rigidbody2DComponent::BodyType::Dynamic;
    auto&& playerComponent = player.AddComponent<Player>();
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/1.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/2.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/3.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/4.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/5.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/6.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/7.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/8.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/9.wav"));
    playerComponent.Audios.push_back(BitPounce::Audio::Create("assets/MirrorDive/Assets/auto/10.wav"));
    playerComponent.onKilledEnemy = gameCallbacks.OnKilledEnemy;
    playerComponent.onWin = gameCallbacks.InAWindow;
    

    AddPlacingItemsCallback([&playerTransform, &cam, &playerComponent](glm::ivec2 pos,std::mt19937& rng, BoundsInt room){
        playerTransform.Translation = room.center();
        playerComponent.rng = rng;
        //cam.Camera.SetOrthographicSize(100);
        return true;
    });

    AddPlacingItemsCallback([scene](glm::ivec2 pos, std::mt19937& rng, BoundsInt room){

        std::uniform_int_distribution distribution(15, 22);
        // Whats 9+10? 21!!!!!!
        if(distribution(rng) != 21)
        {
            return true;
        }

        std::uniform_int_distribution XDistribution(0, 3);
        BitPounce::Entity ent = scene->CreateEntity();
        auto&& Rendor = ent.AddComponent<BitPounce::SpriteRendererComponent>();
        Rendor.SpriteIndex = glm::ivec2(XDistribution(rng), 3);
        Rendor.SpriteSize = glm::ivec2(64);
        Rendor.UseSpriteSheet = true;
        Rendor.Texture = 2865921970978295481;
        auto&& transform = ent.GetComponent<BitPounce::TransformComponent>();
        transform.Translation = glm::vec3(pos.x, pos.y, transform.Translation.z);
        return true;
    });

    AddPlacingItemsCallback([scene, player](glm::ivec2 pos, std::mt19937& rng, BoundsInt room){
        std::uniform_int_distribution distribution(15, 29);
        // Whats 9+10? 21!!!!!!
        int var = distribution(rng);
        if(var != 21 && var != 16 && var != 17 && var != 28)
        {
            return true;
        }
        BitPounce::Entity ent = scene->CreateEntity();
        auto&& transformComponent = ent.GetComponent<BitPounce::TransformComponent>();
        transformComponent.Translation = glm::vec3(pos.x, pos.y, transformComponent.Translation.z);
        auto&& spriteRenderer = ent.AddComponent<BitPounce::SpriteRendererComponent>();
        auto&& enemy = ent.AddComponent<Enemy>();
        enemy.Player = player;
        spriteRenderer.UseSpriteSheet = true;
        spriteRenderer.Colour = glm::vec4(1, 0, 0, 1);
        spriteRenderer.SpriteSize = glm::ivec2(64,64);
        spriteRenderer.Texture = 1993844519112669131;
        spriteRenderer.SpriteIndex = glm::ivec2(0,2);
        if (var == 16)
        {
            // FAST
            enemy.speed = 7.0f;
            spriteRenderer.Colour = glm::vec4(0, 1, 1, 1);
        }
        if (var == 17)
        {
            enemy.speed = 4.f;
            spriteRenderer.Colour = glm::vec4(0.6, 0.6, 0.6, 1);
            auto&& swap = ent.AddComponent<SwapEnemy>();
        }
        return true;
    });
    const uint32_t MAX_WINDOWS = 5;
    uint32_t windowIndex = 0;
    AddPlacingItemsCallback([&windowIndex, scene](glm::ivec2 pos, std::mt19937& rng, BoundsInt room)
    {
        std::uniform_int_distribution distribution(0, 30);
        std::uniform_int_distribution<uint32_t> seedDistribution(0, UINT32_MAX);
        std::uniform_real_distribution<float> timerDistribution(0, 10);

        if(distribution(rng) != 24 || windowIndex >= MAX_WINDOWS)
        {
            return true;
        }
        windowIndex++;
        BitPounce::Entity ent = scene->CreateEntity();
        auto&& transformComponent = ent.GetComponent<BitPounce::TransformComponent>();
        transformComponent.Translation = glm::vec3(pos.x, pos.y, transformComponent.Translation.z);
        auto&& spriteRenderer = ent.AddComponent<BitPounce::SpriteRendererComponent>();
        spriteRenderer.Colour = glm::vec4(1,1,1,1);
        spriteRenderer.Texture = 14759989677347709154;
        auto&& window = ent.AddComponent<Window>();
        window.seed = seedDistribution(rng);
        window.yPos = pos.y;
        window.timer = timerDistribution(rng);

        return true;
    });

    
    BitPounce::Entity ent = GenDungeon(scene, seed);
    playerComponent.tilemap = ent;
    playerRendor.Colour = glm::vec4(1,1,1,1);
    playerRendor.UseSpriteSheet = 1;
    playerRendor.Texture = 14445449532535313356;
    playerRendor.SpriteSize = glm::ivec2(64, 64);
    playerRendor.SpriteIndex = glm::ivec2(0,0);
    scene->OnViewportResize(BitPounce::Application::Get().GetWindow().GetWidth(), BitPounce::Application::Get().GetWindow().GetHeight());
    

    scene->OnRuntimeStart();

    return scene;
}