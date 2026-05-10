#include <BitPounce.h>
#include "Player.h"
#include "Generation.h"

static BitPounce::CameraComponent s_Cam = BitPounce::CameraComponent();

BP_REGISTER_CMD("set_cam_size", "", [](const std::vector<std::string>& args)
{
	float camSize = std::stof(args[0]);
    s_Cam.Camera.SetOrthographicSize(camSize);
});

BitPounce::Ref<BitPounce::Scene> GameLoad()
{
    BitPounce::Ref<BitPounce::Scene> scene = BitPounce::CreateRef<BitPounce::Scene>();
    scene->AddSystem<BitPounce::Renderer2DSystem>();
    scene->AddSystem<BitPounce::CameraSystem>();
    scene->AddSystem<BitPounce::Physics2DSystem>();
    scene->AddSystem<BitPounce::AngelScriptSystem>();
    scene->AddSystem<PlayerSystem>();
    BitPounce::Entity camEnt = scene->CreateEntity("Camera");
    BitPounce::CameraComponent& cam = camEnt.AddComponent<BitPounce::CameraComponent>();
    cam.Camera.SetOrthographicSize(10);
    BitPounce::Entity player = scene->CreateEntity("Player");
    auto&& playerRendor = player.AddComponent<BitPounce::SpriteRendererComponent>();
    auto&& playerRB = player.AddComponent<BitPounce::Rigidbody2DComponent>();
    auto&& playerBox = player.AddComponent<BitPounce::BoxCollider2DComponent>();
    playerRB.Type = BitPounce::Rigidbody2DComponent::BodyType::Dynamic;

    auto&& playerComponent = player.AddComponent<Player>();
    BitPounce::Entity ent = GenDungeon(scene);
    playerComponent.tilemap = ent;
    playerRendor.Colour = glm::vec4(1,1,1,1);
    playerRendor.UseSpriteSheet = 1;
    playerRendor.Texture = 14445449532535313356;
    playerRendor.SpriteSize = glm::ivec2(64, 64);
    playerRendor.SpriteIndex = glm::ivec2(0,0);

    scene->OnRuntimeStart();

    return scene;
}