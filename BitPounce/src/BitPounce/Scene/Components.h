#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "BitPounce/Core/UUID.h"
#include "Entity.h"
#include <BitPounce/Renderer/Texture.h>
#include "BitPounce/Renderer/Font.h"
#include "BitPounce/Renderer/Renderer2D.h"

namespace BitPounce {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(UUID UUID) : ID(UUID) {}
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct ChildrenComponent
	{
		std::vector<Entity> children;
		ChildrenComponent() = default;
		ChildrenComponent(const ChildrenComponent&) = default;
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		Entity Parent = Entity();

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 local = GetLocalTransform();
			if(!Parent)
			{
				return local;
			}

			return Parent.GetComponent<TransformComponent>().GetTransform() * local;
		}

		glm::mat4 GetLocalTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}

		operator const glm::mat4 () const { return GetTransform(); }
	};

	struct TextComponent
	{
		std::string TextString;
		AssetHandle FontHandle = AssetHandle(0);
		//Ref<Font> FontAsset = Font::GetDefault();
		// I can't be bothered, use Renderer2D::TextParams
		Renderer2D::TextParams textParams;
	};

	struct CircleRendererComponent
	{
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		// TODO
		AssetHandle Texture = 0;

		// This does not gets used if UseSpriteSheet is true
		float TilingFactor = 1.0f;

		// This only gets used if UseSpriteSheet is true
		glm::i32vec2 SpriteIndex = glm::i32vec2(0);
		glm::i32vec2 SpriteSize = glm::i32vec2(64);

		bool UseSpriteSheet = false;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& colour)
			: Colour(colour) {}
	};

	struct CameraComponent
	{
		BitPounce::SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;
		glm::vec4 BgColour = glm::vec4(.1f, .1f ,.1f, 1.0f);

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO: move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO: move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	struct DistanceJoint2D
	{
		Entity ConnectedEntity = Entity(entt::null, nullptr);
		bool IsSpring = false;
    	float FrequencyHz = 2.0f;
    	float DampingRatio = 0.5f;
		void* RuntimeJoint = nullptr;
	};

	struct AngelScriptComponent
    {
        std::string ScriptClassName;
        void* ScriptObject = nullptr;
		//entt::entity EntityHandle = entt::null
    };

	// Represents a merged tile rectangle
	struct GreedyQuad
	{
	    glm::vec3 pos;      // world position of min corner
	    glm::vec3 size;     // width, height
	    AssetHandle texture;
	};

	// For BSP node
	struct PartitionNode
	{
	    std::vector<Renderer2D::TileQuad> tiles;
	    std::vector<GreedyQuad> meshes;   // final merged quads in this node
	    std::unique_ptr<PartitionNode> left;
	    std::unique_ptr<PartitionNode> right;
	};


struct GMTile
	{
		glm::mat4 pos;
		// UNUSED AT THE THIS TIME
		std::array<glm::vec2, 4> uvs;
		AssetHandle Texture;
	};

	struct TilemapComponent
	{
		std::vector<GMTile> renderer2D_tiles;
		std::vector<Renderer2D::TileQuad> tiles;
		bool isMod = true;
		void Reserve(size_t size)
		{
			renderer2D_tiles.reserve(size);
			tiles.reserve(size);
			isMod = true;
		}
		void AddTile(const Renderer2D::TileQuad& quad)
		{
			tiles.push_back(quad);
			GMTile gm {};
			gm.pos = glm::mat4(1.0) * glm::translate(glm::mat4(1.0), quad.pos);
			gm.Texture = quad.tex;
			renderer2D_tiles.push_back(gm);
			isMod = true;
		}

		std::vector<GMTile> GetRenderer2DTiles()
		{
			return renderer2D_tiles;
		}

		void RemoveAll()
		{
			renderer2D_tiles.clear();
			tiles.clear();
			isMod = true;
		}
	};

}