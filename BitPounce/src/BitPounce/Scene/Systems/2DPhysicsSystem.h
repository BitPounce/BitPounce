#pragma once

#include <BitPounce/Scene/ECSSystem.h>
#include <BitPounce/Scene/Scene.h>
#include <BitPounce/Core/Timestep.h>
#include <entt/entt.hpp>
#include "../Components.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "BitPounce/ImGui/ImGuiUtils.h"
#include "box2d/box2d.h"

namespace BitPounce
{
	inline b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}
		BP_CORE_ASSERT(false, "Unknown body type");
		return b2_staticBody;
	}

	class Physics2DSystem : public ECSSystem
	{
	public:
		Physics2DSystem() : m_PhysicsWorld(b2_nullWorldId), m_Accumulator(0.0f) {}

		~Physics2DSystem()
		{
			// Ensure world is destroyed and allocated memory freed
			if (b2World_IsValid(m_PhysicsWorld))
				b2DestroyWorld(m_PhysicsWorld);
		}

		virtual void AddComponentPopupImguiDraw(Entity& entity) override
		{
			if (ImGui::MenuItem("Rigidbody2D"))
			{
				entity.AddComponent<Rigidbody2DComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("BoxCollider2D"))
			{
				entity.AddComponent<BoxCollider2DComponent>();
				ImGui::CloseCurrentPopup();
			}
		}

		virtual void OnRuntimeStart() override
		{
			b2WorldDef worldDef = b2DefaultWorldDef();
			worldDef.gravity = {0.0f, -10.0f};
			m_PhysicsWorld = b2CreateWorld(&worldDef);

			auto& registry = m_Scene->GetRegistry(*this);

			registry.view<Rigidbody2DComponent, TransformComponent>().each(
				[&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
				{
					Entity entity{ e, m_Scene };

					// Create Box2D body
					b2BodyDef bodyDef = b2DefaultBodyDef();
					bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
					bodyDef.position = { transform.Translation.x, transform.Translation.y };
					bodyDef.motionLocks.angularZ = rb2d.FixedRotation;

					b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);

					// Store body ID on heap
					rb2d.RuntimeBody = new b2BodyId(bodyId);

					// Create BoxCollider2D if present
					if (entity.HasComponent<BoxCollider2DComponent>())
					{
						auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

						b2Polygon polygon = b2MakeBox(
							bc2d.Size.x * transform.Scale.x,
							bc2d.Size.y * transform.Scale.y
						);

						b2ShapeDef shapeDef = b2DefaultShapeDef();
						shapeDef.density = bc2d.Density;                       // Direct member
						shapeDef.material.friction = bc2d.Friction;
						shapeDef.material.restitution = bc2d.Restitution;
						// restitutionThreshold not available in this Box2D version

						b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);

						// Store shape ID on heap
						bc2d.RuntimeFixture = new b2ShapeId(shapeId);
					}
				});
		}

		virtual void OnRuntimeStop() override
		{
			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			// Free all allocated runtime IDs before destroying the world
			auto& registry = m_Scene->GetRegistry(*this);
			registry.view<Rigidbody2DComponent>().each([&](Rigidbody2DComponent& rb2d)
			{
				if (rb2d.RuntimeBody)
				{
					delete static_cast<b2BodyId*>(rb2d.RuntimeBody);
					rb2d.RuntimeBody = nullptr;
				}
			});
			registry.view<BoxCollider2DComponent>().each([&](BoxCollider2DComponent& bc2d)
			{
				if (bc2d.RuntimeFixture)
				{
					delete static_cast<b2ShapeId*>(bc2d.RuntimeFixture);
					bc2d.RuntimeFixture = nullptr;
				}
			});

			b2DestroyWorld(m_PhysicsWorld);
			m_PhysicsWorld = b2_nullWorldId;
			m_Accumulator = 0.0f;
		}

		virtual void OnUpdate(Timestep& ts) override
		{
			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			// Fixed timestep physics (60 Hz)
			const float fixedTimeStep = 1.0f / 60.0f;
			const int maxSubSteps = 5;
			m_Accumulator += ts;

			int subSteps = 0;
			while (m_Accumulator >= fixedTimeStep && subSteps < maxSubSteps)
			{
				b2World_Step(m_PhysicsWorld, fixedTimeStep, 1); // 1 velocity, 1 position iteration
				m_Accumulator -= fixedTimeStep;
				++subSteps;
			}

			// Update entity transforms from Box2D bodies
			auto& registry = m_Scene->GetRegistry(*this);
			registry.view<Rigidbody2DComponent, TransformComponent>().each(
				[&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
				{
					if (rb2d.RuntimeBody)
					{
						b2BodyId* bodyId = static_cast<b2BodyId*>(rb2d.RuntimeBody);
						b2Vec2 position = b2Body_GetPosition(*bodyId);
						b2Rot rotation = b2Body_GetRotation(*bodyId); 
						float angle = atan2(rotation.s, rotation.c);

						transform.Translation.x = position.x;
						transform.Translation.y = position.y;
						transform.Rotation.z = angle; // Assuming 2D rotation around Z
					}
				});
		}

		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			ImGuiUtils::DrawComponent<Rigidbody2DComponent>("Rigidbody2D", entity, [](Rigidbody2DComponent& rb)
			{
				const char* items[] = { "Static", "Dynamic", "Kinematic" };
				int current = static_cast<int>(rb.Type);
				if (ImGui::Combo("Body Type", &current, items, IM_ARRAYSIZE(items)))
					rb.Type = static_cast<Rigidbody2DComponent::BodyType>(current);

				ImGui::Checkbox("Fixed Rotation", &rb.FixedRotation);
			});

			ImGuiUtils::DrawComponent<BoxCollider2DComponent>("BoxCollider2D", entity, [](BoxCollider2DComponent& bc)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(bc.Offset), 0.01f);
				ImGui::DragFloat2("Size", glm::value_ptr(bc.Size), 0.01f);
				ImGui::DragFloat("Density", &bc.Density, 0.01f);
				ImGui::DragFloat("Friction", &bc.Friction, 0.01f);
				ImGui::DragFloat("Restitution", &bc.Restitution, 0.01f);
				ImGui::DragFloat("Restitution Threshold", &bc.RestitutionThreshold, 0.01f);
			});
		}

	private:
		b2WorldId m_PhysicsWorld;
		float m_Accumulator;
	};
}