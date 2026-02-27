#pragma once

#include "BitPounce.h"

class Sandbox2D : public BitPounce::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(BitPounce::Timestep& ts) override;
	virtual void OnImGuiRender() override;
	void OnDockSpace();
	void OnEvent(BitPounce::Event& e) override;
private:
	BitPounce::OrthographicCameraController m_CameraController;
	float m_time = 0.0f;

	BitPounce::Ref<BitPounce::Texture2D> m_PlayerTexture;
	BitPounce::Ref<BitPounce::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};