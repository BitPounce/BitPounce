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
			: m_PhysicsWorld(b2_nullWorldId), m_Accumulator(0.0f), m_Gravity(0.0f, 0.0f) {m_name = "2D Physics System";}

		~Physics2DSystem()
		{
			if (b2World_IsValid(m_PhysicsWorld))
				b2DestroyWorld(m_PhysicsWorld);
		}

		Physics2DSystem(const Physics2DSystem& other)
			: m_PhysicsWorld(b2_nullWorldId), m_Accumulator(0.0f), m_Gravity(other.m_Gravity) {m_name = "2D Physics System";}

		Physics2DSystem* clone() const override {
			return new Physics2DSystem(*this);
		}

		// ---------------------------------------------------------------------
		// ImGui editor drawing
		// ---------------------------------------------------------------------
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

			// DistanceJoint2D (Length removed)
			if (entity.HasComponent<DistanceJoint2D>())
			{
				ImGuiUtils::DrawComponent<DistanceJoint2D>("DistanceJoint2D", entity, [this](DistanceJoint2D& dj)
				{
					// Entity selection - simplified: show entity ID, allow manual input
					//uint32_t connectedID = (uint32_t)dj.ConnectedEntity;
					//if (ImGui::DragInt("Connected Entity ID", (int*)&connectedID, 1.0f, 0, INT_MAX))
					//{
					//	auto& registry = m_Scene->GetRegistry(*this);
					//	for (auto [e, tag] : registry.view<TagComponent>().each())
					//	{
					//		if ((uint32_t)e == connectedID)
					//		{
					//			dj.ConnectedEntity = Entity{ e, m_Scene };
					//			break;
					//		}
					//	}
					//}

					Entity ent = ImGuiUtils::DrawEntitySelect("Connected Entity");
					if(ent)
					{
						dj.ConnectedEntity = ent;
					}
					if(dj.ConnectedEntity)
					{
						ImGui::Text((std::string("Connected Entity Name:" ) + dj.ConnectedEntity.GetName()).c_str());
					}

					ImGui::Checkbox("Is Spring", &dj.IsSpring);
					if (dj.IsSpring)
					{
						ImGui::DragFloat("Frequency (Hz)", &dj.FrequencyHz, 0.1f, 0.0f, 100.0f);
						ImGui::DragFloat("Damping Ratio", &dj.DampingRatio, 0.01f, 0.0f, 2.0f);
					}
				});
			}

			// System-wide settings
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
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["Rigidbody2D"] = rbJson;
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
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["BoxCollider2D"] = bcJson;
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
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["CircleCollider2D"] = ccJson;
			}

			// DistanceJoint2D (Length removed)
			auto djView = registry.view<DistanceJoint2D>();
			for (auto entity : djView)
			{
				auto& dj = djView.get<DistanceJoint2D>(entity);
				nlohmann::json djJson;
				djJson["ConnectedEntity"] = (uint64_t)dj.ConnectedEntity.GetUUID();
				djJson["IsSpring"] = dj.IsSpring;
				djJson["FrequencyHz"] = dj.FrequencyHz;
				djJson["DampingRatio"] = dj.DampingRatio;
				// Length field no longer serialized
				for (auto& ent : json["Entities"])
					if (ent["entityID"].get<uint32_t>() == (uint32_t)entity)
						ent["DistanceJoint2D"] = djJson;
			}

			// System settings
			json["Physics2D"]["Gravity"] = { m_Gravity.x, m_Gravity.y };
		}

		virtual void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
		    ECSSystem::CopyComponentBASE<DistanceJoint2D>(dst, src, enttMap);
		
		    auto view = dst.view<DistanceJoint2D>();
		
		    for (auto entity : view)
		    {
		        auto& joint = view.get<DistanceJoint2D>(entity);
			
		        if (!joint.ConnectedEntity)
		            continue;
			
		        UUID connectedUUID = joint.ConnectedEntity.GetUUID();
			
		        joint.ConnectedEntity = m_Scene->FindEntityByUUID(connectedUUID);
		    }
		}

		virtual void Deserialize(nlohmann::json& json) override
		{
			auto& registry = m_Scene->GetRegistry(*this);

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
				if (!entJson.contains("entityID")) continue;
				uint32_t id = entJson["entityID"].get<uint32_t>();
				if (!entityMap.contains(id)) continue;
				Entity entity{ entityMap[id], m_Scene };

				// Rigidbody2D
				if (entJson.contains("Rigidbody2D"))
				{
					auto& rbJson = entJson["Rigidbody2D"];
					Rigidbody2DComponent rb;
					if (rbJson.contains("Type")) rb.Type = (Rigidbody2DComponent::BodyType)rbJson["Type"].get<int>();
					if (rbJson.contains("FixedRotation")) rb.FixedRotation = rbJson["FixedRotation"].get<bool>();
					if (!entity.HasComponent<Rigidbody2DComponent>()) entity.AddComponent<Rigidbody2DComponent>(rb);
				}

				// BoxCollider2D
				if (entJson.contains("BoxCollider2D"))
				{
					auto& bcJson = entJson["BoxCollider2D"];
					BoxCollider2DComponent bc;
					if (bcJson.contains("Offset")) { auto o = bcJson["Offset"]; bc.Offset = { o[0], o[1] }; }
					if (bcJson.contains("Size")) { auto s = bcJson["Size"]; bc.Size = { s[0], s[1] }; }
					if (bcJson.contains("Density")) bc.Density = bcJson["Density"].get<float>();
					if (bcJson.contains("Friction")) bc.Friction = bcJson["Friction"].get<float>();
					if (bcJson.contains("Restitution")) bc.Restitution = bcJson["Restitution"].get<float>();
					if (bcJson.contains("RestitutionThreshold")) bc.RestitutionThreshold = bcJson["RestitutionThreshold"].get<float>();
					if (!entity.HasComponent<BoxCollider2DComponent>()) entity.AddComponent<BoxCollider2DComponent>(bc);
				}

				// CircleCollider2D
				if (entJson.contains("CircleCollider2D"))
				{
					auto& ccJson = entJson["CircleCollider2D"];
					CircleCollider2DComponent cc;
					if (ccJson.contains("Offset")) { auto o = ccJson["Offset"]; cc.Offset = { o[0], o[1] }; }
					if (ccJson.contains("Radius")) cc.Radius = ccJson["Radius"].get<float>();
					if (ccJson.contains("Density")) cc.Density = ccJson["Density"].get<float>();
					if (ccJson.contains("Friction")) cc.Friction = ccJson["Friction"].get<float>();
					if (ccJson.contains("Restitution")) cc.Restitution = ccJson["Restitution"].get<float>();
					if (ccJson.contains("RestitutionThreshold")) cc.RestitutionThreshold = ccJson["RestitutionThreshold"].get<float>();
					if (!entity.HasComponent<CircleCollider2DComponent>()) entity.AddComponent<CircleCollider2DComponent>(cc);
				}

				// DistanceJoint2D (Length removed)
				if (entJson.contains("DistanceJoint2D"))
				{
					auto& djJson = entJson["DistanceJoint2D"];
					DistanceJoint2D dj;
					if (djJson.contains("ConnectedEntity"))
					{
						uint64_t connectedID = djJson["ConnectedEntity"].get<uint64_t>();
						dj.ConnectedEntity = m_Scene->FindEntityByUUID(connectedID);
					}
					if (djJson.contains("IsSpring")) dj.IsSpring = djJson["IsSpring"].get<bool>();
					if (djJson.contains("FrequencyHz")) dj.FrequencyHz = djJson["FrequencyHz"].get<float>();
					if (djJson.contains("DampingRatio")) dj.DampingRatio = djJson["DampingRatio"].get<float>();
					// Length field no longer deserialized
					if (!entity.HasComponent<DistanceJoint2D>()) entity.AddComponent<DistanceJoint2D>(dj);
				}
			}
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
			if (ImGui::MenuItem("DistanceJoint2D"))
			{
				entity.AddComponent<DistanceJoint2D>();
				ImGui::CloseCurrentPopup();
			}
		}

		virtual void OnRuntimeStart() override
		{
			b2WorldDef worldDef = b2DefaultWorldDef();
			worldDef.gravity = { m_Gravity.x, m_Gravity.y };
			m_PhysicsWorld = b2CreateWorld(&worldDef);
			m_IsInRuntime = true;

			auto& registry = m_Scene->GetRegistry(*this);

			// Create rigidbodies and colliders
			registry.view<Rigidbody2DComponent, TransformComponent>().each([&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
				Entity entity{ e, m_Scene };
				CreateRigidbody(entity);
			});

			// Create distance joints (must come after bodies exist)
			registry.view<DistanceJoint2D>().each([&](entt::entity e, DistanceJoint2D& joint)
			{
				Entity entity{ e, m_Scene };
				CreateDistanceJoint(entity);
			});
		}

		virtual void OnRuntimeStop() override
		{
			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			auto& registry = m_Scene->GetRegistry(*this);

			// Destroy all joints first
			registry.view<DistanceJoint2D>().each([&](DistanceJoint2D& joint)
			{
				DestroyDistanceJoint(joint);
			});

			// Clean up rigidbody runtime data
			registry.view<Rigidbody2DComponent>().each([&](Rigidbody2DComponent& rb2d)
			{
				delete static_cast<b2BodyId*>(rb2d.RuntimeBody);
				rb2d.RuntimeBody = nullptr;
			});

			// Clean up colliders
			registry.view<BoxCollider2DComponent>().each([&](BoxCollider2DComponent& bc2d)
			{
				delete static_cast<b2ShapeId*>(bc2d.RuntimeFixture);
				bc2d.RuntimeFixture = nullptr;
			});
			registry.view<CircleCollider2DComponent>().each([&](CircleCollider2DComponent& cc2d)
			{
				delete static_cast<b2ShapeId*>(cc2d.RuntimeFixture);
				cc2d.RuntimeFixture = nullptr;
			});

			b2DestroyWorld(m_PhysicsWorld);
			m_PhysicsWorld = b2_nullWorldId;
			m_Accumulator = 0.0f;
			m_IsInRuntime = false;
		}

		virtual void OnUpdate(Timestep& ts) override
		{
			if (!b2World_IsValid(m_PhysicsWorld))
				return;

			// Make sure all components have runtime Box2D objects (for dynamic addition)
			EnsureRuntimeComponents();

			auto& registry = m_Scene->GetRegistry(*this);

			// Sync transforms -> Box2D bodies
			registry.view<Rigidbody2DComponent, TransformComponent>().each(
			[&](auto e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
				if (rb2d.RuntimeBody)
				{
					glm::mat4 mat = transform.GetTransform();
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(mat, translation, rotation, scale);
					b2Body_SetTransform(*(b2BodyId*)(rb2d.RuntimeBody), {translation.x, translation.y}, b2MakeRot(rotation.z));
					// AWAKE UP!
					b2Body_SetAwake(*(b2BodyId*)(rb2d.RuntimeBody), true);
				}
			});

			// Physics step
			const float fixedTimeStep = 1.0f / 50.0f;
			const int maxSubSteps = 5;
			m_Accumulator += ts;

			int subSteps = 0;
			while (m_Accumulator >= fixedTimeStep && subSteps < maxSubSteps)
			{
				b2World_Step(m_PhysicsWorld, fixedTimeStep, 1);
				m_Accumulator -= fixedTimeStep;
				++subSteps;
			}

			// Update transforms from Box2D (non-static bodies)
			registry.view<Rigidbody2DComponent, TransformComponent>().each([&](entt::entity e, Rigidbody2DComponent& rb2d, TransformComponent& transform)
			{
				if (!rb2d.RuntimeBody) return;
				b2BodyId bodyId = *static_cast<b2BodyId*>(rb2d.RuntimeBody);
				if (!b2Body_IsValid(bodyId)) return;

				b2Vec2 worldPos = b2Body_GetPosition(bodyId);
				b2Rot worldRot = b2Body_GetRotation(bodyId);
				float worldAngle = atan2(worldRot.s, worldRot.c);

				if (transform.Parent)
				{
					glm::mat4 parentWorld = transform.Parent.GetComponent<TransformComponent>().GetTransform();
					glm::mat4 worldTransform = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos.x, worldPos.y, transform.Translation.z))
											 * glm::toMat4(glm::quat(glm::vec3(0.0f, 0.0f, worldAngle)))
											 * glm::scale(glm::mat4(1.0f), transform.Scale);
					glm::mat4 localTransform = glm::inverse(parentWorld) * worldTransform;
					glm::vec3 localTranslation, localRotation, localScale;
					Math::DecomposeTransform(localTransform, localTranslation, localRotation, localScale);
					transform.Translation = localTranslation;
					transform.Rotation = localRotation;
					transform.Scale = localScale;
				}
				else
				{
					transform.Translation.x = worldPos.x;
					transform.Translation.y = worldPos.y;
					transform.Rotation = glm::vec3(0.0f, 0.0f, worldAngle);
				}
			});
		}

	private:
		void CreateRigidbody(Entity entity)
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			auto& transform = entity.GetComponent<TransformComponent>();

			glm::mat4 mat = transform.GetTransform();
			glm::vec3 translation, rotation, scale;
			Math::DecomposeTransform(mat, translation, rotation, scale);

			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position = { translation.x, translation.y };
			bodyDef.rotation = b2MakeRot(rotation.z);
			bodyDef.motionLocks.angularZ = rb2d.FixedRotation;

			b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);
			rb2d.RuntimeBody = new b2BodyId(bodyId);

			if (entity.HasComponent<BoxCollider2DComponent>())
				CreateBoxColliderFixture(entity, bodyId, scale);
			if (entity.HasComponent<CircleCollider2DComponent>())
				CreateCircleColliderFixture(entity, bodyId, scale);
		}

		void CreateBoxColliderFixture(Entity entity, b2BodyId bodyId, const glm::vec3& scale)
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
			if (bc2d.RuntimeFixture) return;

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

		void CreateCircleColliderFixture(Entity entity, b2BodyId bodyId, const glm::vec3& scale)
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
			if (cc2d.RuntimeFixture) return;

			float radius = scale.x * cc2d.Radius; // assuming uniform scale
			b2Circle circle = { { cc2d.Offset.x, cc2d.Offset.y }, radius };

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = cc2d.Density;
			shapeDef.material.friction = cc2d.Friction;
			shapeDef.material.restitution = cc2d.Restitution;

			b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
			cc2d.RuntimeFixture = new b2ShapeId(shapeId);
		}

		void CreateDistanceJoint(Entity entity)
		{
			auto& joint = entity.GetComponent<DistanceJoint2D>();
			if (joint.RuntimeJoint) return;

			if (!joint.ConnectedEntity || !joint.ConnectedEntity.HasComponent<Rigidbody2DComponent>())
			{
				BP_CORE_WARN("DistanceJoint2D on entity {} has invalid ConnectedEntity", (uint32_t)entity);
				return;
			}

			auto& bodyA = entity.GetComponent<Rigidbody2DComponent>();
			auto& bodyB = Entity((entt::entity)0, m_Scene).GetComponent<Rigidbody2DComponent>();

			if (!bodyA.RuntimeBody || !bodyB.RuntimeBody) return;

			b2BodyId bodyIdA = *static_cast<b2BodyId*>(bodyA.RuntimeBody);
			b2BodyId bodyIdB = *static_cast<b2BodyId*>(bodyB.RuntimeBody);

			// Length is always the current distance between bodies
			b2Vec2 posA = b2Body_GetPosition(bodyIdA);
			b2Vec2 posB = b2Body_GetPosition(bodyIdB);
			float distance = b2Distance(posA, posB);

			b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
			jointDef.base.bodyIdA = bodyIdA;
			jointDef.base.bodyIdB = bodyIdB;
			jointDef.length = distance;
			jointDef.minLength = distance;
			jointDef.maxLength = distance;

			if (joint.IsSpring)
			{
				jointDef.enableSpring = true;
				jointDef.hertz = joint.FrequencyHz;
				jointDef.dampingRatio = joint.DampingRatio;
			}
			else
			{
				jointDef.enableSpring = false;
			}

			b2JointId jointId = b2CreateDistanceJoint(m_PhysicsWorld, &jointDef);
			joint.RuntimeJoint = new b2JointId(jointId);
		}

		void DestroyDistanceJoint(DistanceJoint2D& joint)
		{
			if (joint.RuntimeJoint)
			{
				b2JointId jointId = *static_cast<b2JointId*>(joint.RuntimeJoint);
				if (b2Joint_IsValid(jointId))
					b2DestroyJoint(jointId, true);
				delete static_cast<b2JointId*>(joint.RuntimeJoint);
				joint.RuntimeJoint = nullptr;
			}
		}

		void EnsureRuntimeComponents()
		{
			auto& registry = m_Scene->GetRegistry(*this);

			// 1. Rigidbodies
			auto rbView = registry.view<Rigidbody2DComponent, TransformComponent>();
			for (auto e : rbView)
			{
				auto& rb2d = rbView.get<Rigidbody2DComponent>(e);
				if (!rb2d.RuntimeBody)
				{
					Entity entity{ e, m_Scene };
					CreateRigidbody(entity);
				}
			}

			// 2. Box colliders (ensure fixture)
			auto boxView = registry.view<BoxCollider2DComponent>();
			for (auto e : boxView)
			{
				auto& bc2d = boxView.get<BoxCollider2DComponent>(e);
				if (bc2d.RuntimeFixture) continue;

				Entity entity{ e, m_Scene };
				if (!entity.HasComponent<Rigidbody2DComponent>())
					entity.AddComponent<Rigidbody2DComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
				if (!rb2d.RuntimeBody)
					CreateRigidbody(entity);
				b2BodyId bodyId = *static_cast<b2BodyId*>(rb2d.RuntimeBody);
				glm::vec3 scale = entity.GetComponent<TransformComponent>().Scale;
				CreateBoxColliderFixture(entity, bodyId, scale);
			}

			// 3. Circle colliders
			auto circleView = registry.view<CircleCollider2DComponent>();
			for (auto e : circleView)
			{
				auto& cc2d = circleView.get<CircleCollider2DComponent>(e);
				if (cc2d.RuntimeFixture) continue;

				Entity entity{ e, m_Scene };
				if (!entity.HasComponent<Rigidbody2DComponent>())
					entity.AddComponent<Rigidbody2DComponent>();
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
				if (!rb2d.RuntimeBody)
					CreateRigidbody(entity);
				b2BodyId bodyId = *static_cast<b2BodyId*>(rb2d.RuntimeBody);
				glm::vec3 scale = entity.GetComponent<TransformComponent>().Scale;
				CreateCircleColliderFixture(entity, bodyId, scale);
			}

			// 4. Distance joints
			auto djView = registry.view<DistanceJoint2D>();
			for (auto e : djView)
			{
				auto& joint = djView.get<DistanceJoint2D>(e);
				if (!joint.RuntimeJoint)
				{
					Entity entity{ e, m_Scene };
					if (joint.ConnectedEntity && !joint.ConnectedEntity.HasComponent<Rigidbody2DComponent>())
						joint.ConnectedEntity.AddComponent<Rigidbody2DComponent>();
					CreateDistanceJoint(entity);
				}
			}
		}

	private:
		bool m_IsInRuntime = false;
		b2WorldId m_PhysicsWorld;
		float m_Accumulator;
		glm::vec2 m_Gravity;
		friend class ECSSystem;
	};
}