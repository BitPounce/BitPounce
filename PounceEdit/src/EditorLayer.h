#pragma once

#include "BitPounce.h"
namespace BitPounce {
class EditorLayer : public BitPounce::Layer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(BitPounce::Timestep& ts) override;
	virtual void OnImGuiRender() override;
	void OnDockSpace();
	void OnEvent(BitPounce::Event& e) override;

	
private:
	BitPounce::SystemManager m_SysManager;
	BitPounce::OrthographicCameraController m_CameraController;
	float m_time = 0.0f;

	BitPounce::Ref<BitPounce::Texture2D> m_PlayerTexture;
	BitPounce::Ref<BitPounce::Texture2D> m_Icon;
	BitPounce::Ref<BitPounce::Texture2D> m_CheckerboardTexture;
	BitPounce::Ref<BitPounce::Framebuffer> m_Framebuffer;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec2 m_RendorSize = {};
};

}