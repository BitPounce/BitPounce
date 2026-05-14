#pragma once
#include <BitPounce.h>

class ECSTest: public BitPounce::Layer
{

public:
	ECSTest();
	virtual ~ECSTest() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(BitPounce::Timestep& ts) override;
	virtual void OnImGuiRender(BitPounce::Timestep& ts) override;
	void OnEvent(BitPounce::Event& e) override;
	void DrawMainMenu(BitPounce::Timestep& ts);
	void DrawSetingsMenu(BitPounce::Timestep& ts);
	
private:
	uint32_t m_Score = 0;
	uint32_t m_HighScore = 0;
	bool m_IsInAWindow = false;
	bool m_IsSettingsWindowOpen = false;
	bool m_PlayerHasLost = false;
	uint32_t seed = 466456;
	BitPounce::Ref<BitPounce::Project> m_Project;
	BitPounce::Ref<BitPounce::Scene> m_Scene;
	BitPounce::ImGuiConsoleWindow m_Console;
	BitPounce::OrthographicCamera m_Camera;
	BitPounce::Ref<BitPounce::Framebuffer> m_Framebuffer;
	BitPounce::Ref<BitPounce::Shader> m_Shader;
	BitPounce::Ref<BitPounce::Audio> m_MainAudio;
	bool OnAssetPreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnScenePreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnWindowResize(BitPounce::WindowResizeEvent& e);

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
