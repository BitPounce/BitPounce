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
#include <unordered_map>
#include "BitPounce/Math/Math.h"

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
		Physics2DSystem()
			: m_PhysicsWorld(b2_nullWorldId), m_Accumulator(0.0f), m_Gravity(0.0f, -10.0f) {}

		~Physics2DSystem()
		{
			if (b2World_IsValid(m_PhysicsWorld))
				b2DestroyWorld(m_PhysicsWorld);
		}

		Physics2DSystem(const Physics2DSystem& other)
			: m_PhysicsWorld(b2_nullWorldId), m_Accumulator(0.0f), m_Gravity(other.m_Gravity) {}

		Physics2DSystem* clone() const override {
			return new Physics2DSystem(*this);
		}

		virtual void OnEditorPropImguiDraw(Entity& entity) override
		{
			// Rigidbody2D
			if (entity.HasComponent<Rigidbody2DComponent>())
			{
				ImGuiUtils::DrawComponent<Rigidbody2DComponent>("Rigidbody2D", entity, [](Rigidbody2DComponent& rb)
				{
					const char* items[] = { "Static", "Dynamic", "Kinematic" };
					int current = static_cast<int>(rb.Type);
					if (ImGui::Combo("Body Type", &current, items, IM_ARRAYSIZE(items)))
						rb.Type = static_cast<Rigidbody2DComponent::BodyType>(current);
					ImGui::Checkbox("Fixed Rotation", &rb.FixedRotation);
				});
			}

			// BoxCollider2D
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				ImGuiUtils::DrawComponent<BoxCollider2DComponent>("BoxCollider2D", entity, [](BoxCollider2DComponent& bc)
				{
					ImGui::DragFloat2("Offset", glm::value_ptr(bc.Offset), 0.01f);
					ImGui::DragFloat2("Size", glm::value_ptr(bc.Size), 0.01f, 0.01f);
					ImGui::DragFloat("Density", &bc.Density, 0.01f, 0.0f);
					ImGui::DragFloat("Friction", &bc.Friction, 0.01f, 0.0f);
					ImGui::DragFloat("Restitution", &bc.Restitution, 0.01f, 0.0f);
					ImGui::DragFloat("Restitution Threshold", &bc.RestitutionThreshold, 0.01f, 0.0f);
				});
			}

			// CircleCollider2D
			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				ImGuiUtils::DrawComponent<CircleCollider2DComponent>("CircleCollider2D", entity, [](CircleCollider2DComponent& cc)
				{
					ImGui::DragFloat2("Offset", glm::value_ptr(cc.Offset), 0.01f);
					ImGui::DragFloat("Radius", &cc.Radius, 0.01f, 0.01f);
					ImGui::DragFloat("Density", &cc.Density, 0.01f, 0.0f);
					ImGui::DragFloat("Friction", &cc.Friction, 0.01f, 0.0f);
					ImGui::DragFloat("Restitution", &cc.Restitution, 0.01f, 0.0f);
					ImGui::DragFloat("Restitution Threshold", &cc.RestitutionThreshold, 0.01f, 0.0f);
				});
			}

			// Optional: System-wide settings (Gravity)
			if (ImGui::CollapsingHeader("Physics2D System Settings"))
			{
				ImGui::DragFloat2("Gravity", glm::value_ptr(m_Gravity), 0.1f);
			}
		}

		virtual void Serialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			// Rigidbody2D
			auto rbView = registry.view<Rigidbody2DComponent>();
			for (auto entity : rbView)
			{
				auto& rb = rbView.get<Rigidbody2DComponent>(entity);

				nlohmann::json rbJson;
				rbJson["Type"] = (int)rb.Type;
				rbJson["FixedRotation"] = rb.FixedRotation;

				for (auto& ent : json["Entities"])
				{
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
					{
						ent["Rigidbody2D"] = rbJson;
						break;
					}
				}
			}

			// BoxCollider2D
			auto bcView = registry.view<BoxCollider2DComponent>();
			for (auto entity : bcView)
			{
				auto& bc = bcView.get<BoxCollider2DComponent>(entity);

				nlohmann::json bcJson;
				bcJson["Offset"] = { bc.Offset.x, bc.Offset.y };
				bcJson["Size"] = { bc.Size.x, bc.Size.y };
				bcJson["Density"] = bc.Density;
				bcJson["Friction"] = bc.Friction;
				bcJson["Restitution"] = bc.Restitution;
				bcJson["RestitutionThreshold"] = bc.RestitutionThreshold;

				for (auto& ent : json["Entities"])
				{
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
					{
						ent["BoxCollider2D"] = bcJson;
						break;
					}
				}
			}

			// CircleCollider2D
			auto ccView = registry.view<CircleCollider2DComponent>();
			for (auto entity : ccView)
			{
				auto& cc = ccView.get<CircleCollider2DComponent>(entity);

				nlohmann::json ccJson;
				ccJson["Offset"] = { cc.Offset.x, cc.Offset.y };
				ccJson["Radius"] = cc.Radius;
				ccJson["Density"] = cc.Density;
				ccJson["Friction"] = cc.Friction;
				ccJson["Restitution"] = cc.Restitution;
				ccJson["RestitutionThreshold"] = cc.RestitutionThreshold;

				for (auto& ent : json["Entities"])
				{
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
					{
						ent["CircleCollider2D"] = ccJson;
						break;
					}
				}
			}

			// System settings
			json["Physics2D"]["Gravity"] = { m_Gravity.x, m_Gravity.y };
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

			if (ImGui::MenuItem("CircleCollider2D"))
			{
				entity.AddComponent<CircleCollider2DComponent>();
				ImGui::CloseCurrentPopup();
			}
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

			// Gravity
			if (json.contains("Physics2D"))
			{
				auto& phys = json["Physics2D"];
				if (phys.contains("Gravity"))
				{
					auto g = phys["Gravity"];
					m_Gravity = { g[0], g[1] };
				}
			}

			// Build entity map
			std::unordered_map<uint32_t, entt::entity> entityMap;

			auto view = registry.view<TagComponent>();
			for (auto entity : view)
			{
				Entity e{ entity, m_Scene };
				entityMap[(uint32_t)e] = entity;
			}

			// Deserialize components
			for (auto& entJson : json["Entities"])
			{
				if (!entJson.contains("entityID"))
					continue;

				uint32_t id = entJson["entityID"].get<uint32_t>();

				if (!entityMap.contains(id))
					continue;

				Entity entity{ entityMap[id], m_Scene };

				// Rigidbody2D
				if (entJson.contains("Rigidbody2D"))
				{
					auto& rbJson = entJson["Rigidbody2D"];

					Rigidbody2DComponent rb;

					if (rbJson.contains("Type"))
						rb.Type = (Rigidbody2DComponent::BodyType)rbJson["Type"].get<int>();

					if (rbJson.contains("FixedRotation"))
						rb.FixedRotation = rbJson["FixedRotation"].get<bool>();

					if (!entity.HasComponent<Rigidbody2DComponent>())
						entity.AddComponent<Rigidbody2DComponent>(rb);
				}

				// BoxCollider2D
				if (entJson.contains("BoxCollider2D"))
				{
					auto& bcJson = entJson["BoxCollider2D"];

					BoxCollider2DComponent bc;

					if (bcJson.contains("Offset"))
					{
						auto o = bcJson["Offset"];
						bc.Offset = { o[0], o[1] };
					}

					if (bcJson.contains("Size"))
					{
						auto s = bcJson["Size"];
						bc.Size = { s[0], s[1] };
					}

					if (bcJson.contains("Density"))
						bc.Density = bcJson["Density"].get<float>();

					if (bcJson.contains("Friction"))
						bc.Friction = bcJson["Friction"].get<float>();

					if (bcJson.contains("Restitution"))
						bc.Restitution = bcJson["Restitution"].get<float>();

					if (bcJson.contains("RestitutionThreshold"))
						bc.RestitutionThreshold = bcJson["RestitutionThreshold"].get<float>();

					if (!entity.HasComponent<BoxCollider2DComponent>())
						entity.AddComponent<BoxCollider2DComponent>(bc);
				}

				// CircleCollider2D
				if (entJson.contains("CircleCollider2D"))
				{
					auto& ccJson = entJson["CircleCollider2D"];

					CircleCollider2DComponent cc;

					if (ccJson.contains("Offset"))
					{
						auto o = ccJson["Offset"];
						cc.Offset = { o[0], o[1] };
					}

					if (ccJson.contains("Radius"))
						cc.Radius = ccJson["Radius"].get<float>();

					if (ccJson.contains("Density"))
						cc.Density = ccJson["Density"].get<float>();

					if (ccJson.contains("Friction"))
						cc.Friction = ccJson["Friction"].get<float>();

					if (ccJson.contains("Restitution"))
						cc.Restitution = ccJson["Restitution"].get<float>();

					if (ccJson.contains("RestitutionThreshold"))
						cc.RestitutionThreshold = ccJson["RestitutionThreshold"].get<float>();

					if (!entity.HasComponent<CircleCollider2DComponent>())
						entity.AddComponent<CircleCollider2DComponent>(cc);
				}
			}
		}

		virtual void OnRuntimeStart() override
		{
			b2WorldDef worldDef = b2DefaultWorldDef();
			worldDef.gravity = { m_Gravity.x, m_Gravity.y };
			m_PhysicsWorld = b2CreateWorld(&worldDef);

			auto& registry = m_Scene->GetRegistry(*this);

			registry.view<Rigidbody2DComponent, TransformComponent>().each([&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
				Entity entity{ e, m_Scene };

				b2BodyDef bodyDef = b2DefaultBodyDef();
				bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
				glm::mat4 mat = transform.GetTransform();
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(mat, translation, rotation, scale);
				bodyDef.position = { translation.x, translation.y };
				bodyDef.rotation = b2MakeRot(rotation.z);
				bodyDef.motionLocks.angularZ = rb2d.FixedRotation;

				b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);
				rb2d.RuntimeBody = new b2BodyId(bodyId);

				// Box collider
				if (entity.HasComponent<BoxCollider2DComponent>())
				{
					auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

					float hx = bc2d.Size.x * scale.x;
					float hy = bc2d.Size.y * scale.y;
					b2Polygon polygon = b2MakeBox(hx, hy);

					b2ShapeDef shapeDef = b2DefaultShapeDef();
					shapeDef.density = bc2d.Density;
					shapeDef.material.friction = bc2d.Friction;
					shapeDef.material.restitution = bc2d.Restitution;

					b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
					bc2d.RuntimeFixture = new b2ShapeId(shapeId);
				}

				// Circle collider
				if (entity.HasComponent<CircleCollider2DComponent>())
				{
					auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

					// Circle radius scaled by transform scale (assuming uniform scale)
					float radius = scale.x * cc2d.Radius;
					b2Circle circle = { { cc2d.Offset.x, cc2d.Offset.y }, radius };

					b2ShapeDef shapeDef = b2DefaultShapeDef();
					shapeDef.density = cc2d.Density;
					shapeDef.material.friction = cc2d.Friction;
					shapeDef.material.restitution = cc2d.Restitution;

					b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
					cc2d.RuntimeFixture = new b2ShapeId(shapeId);
				}
			});
		}

		virtual void OnRuntimeStop() override
		{
			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			auto& registry = m_Scene->GetRegistry(*this);

			// Clean up rigidbody runtime data
			registry.view<Rigidbody2DComponent>().each([&](Rigidbody2DComponent& rb2d)
			{
				delete static_cast<b2BodyId*>(rb2d.RuntimeBody);
				rb2d.RuntimeBody = nullptr;
			});

			// Clean up box collider runtime data
			registry.view<BoxCollider2DComponent>().each([&](BoxCollider2DComponent& bc2d)
			{
				delete static_cast<b2ShapeId*>(bc2d.RuntimeFixture);
				bc2d.RuntimeFixture = nullptr;
			});

			// Clean up circle collider runtime data
			registry.view<CircleCollider2DComponent>().each([&](CircleCollider2DComponent& cc2d)
			{
				delete static_cast<b2ShapeId*>(cc2d.RuntimeFixture);
				cc2d.RuntimeFixture = nullptr;
			});

			b2DestroyWorld(m_PhysicsWorld);
			m_PhysicsWorld = b2_nullWorldId;
			m_Accumulator = 0.0f;
		}

		virtual void OnUpdate(Timestep& ts) override
		{

			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			auto& registry = m_Scene->GetRegistry(*this);
			registry.view<Rigidbody2DComponent, TransformComponent>().each(
			[&](auto e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
				if (rb2d.RuntimeBody)
				{
					glm::mat4 mat = transform.GetTransform();
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(mat, translation, rotation, scale);

					auto bodyId = static_cast<b2BodyId*>(rb2d.RuntimeBody);
					b2Body_SetTransform(*(b2BodyId*)(rb2d.RuntimeBody), {translation.x, translation.y}, b2MakeRot(rotation.z));
				}
			});

			const float fixedTimeStep = 1.0f / 60.0f;
			const int maxSubSteps = 5;
			m_Accumulator += ts;

			int subSteps = 0;
			while (m_Accumulator >= fixedTimeStep && subSteps < maxSubSteps)
			{
				b2World_Step(m_PhysicsWorld, fixedTimeStep, 1);
				m_Accumulator -= fixedTimeStep;
				++subSteps;
			}

			
			registry.view<Rigidbody2DComponent, TransformComponent>().each([&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
			    if (!rb2d.RuntimeBody) return;
			    b2BodyId bodyId = *static_cast<b2BodyId*>(rb2d.RuntimeBody);
			    if (!b2Body_IsValid(bodyId)) return;
			
			    // 1. Get world position/rotation from Box2D
			    b2Vec2 worldPos = b2Body_GetPosition(bodyId);
			    b2Rot worldRot = b2Body_GetRotation(bodyId);
			    float worldAngle = atan2(worldRot.s, worldRot.c);
			
			    // 2. Convert to local space if entity has a parent
			    if (transform.Parent)
			    {
			        // Get parent's world transform
			        glm::mat4 parentWorld = transform.Parent.GetComponent<TransformComponent>().GetTransform();
				
			        // Build this entity's world transform from Box2D data
			        glm::mat4 worldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, transform.Translation.z))
			                                 * glm::toMat4(glm::quat(glm::vec3(0.0f, 0.0f, worldAngle)))
			                                 * glm::scale(glm::mat4(1.0f), transform.Scale);
				
			        // Convert to local space relative to parent
			        glm::mat4 localTransform = glm::inverse(parentWorld) * worldTransform;
				
			        // Decompose local transform into translation, rotation, scale
			        // (You need a DecomposeTransform function that extracts from a matrix)
			        glm::vec3 localTranslation, localRotation, localScale;
			        Math::DecomposeTransform(localTransform, localTranslation, localRotation, localScale);
				
			        transform.Translation = localTranslation;
			        transform.Rotation = localRotation;
			        transform.Scale = localScale; // scale unchanged, but keep it
			    }
			    else
			    {
			        // No parent: assign world values directly to local transform
			        transform.Translation.x = worldPos.x;
			        transform.Translation.y = worldPos.y;
			        // Keep original Z
			        transform.Rotation = glm::vec3(0.0f, 0.0f, worldAngle);
			    }
			});
		}

	private:
		b2WorldId m_PhysicsWorld;
		float m_Accumulator;
		glm::vec2 m_Gravity;
	};
}