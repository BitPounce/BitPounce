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

	
private:
	BitPounce::Ref<BitPounce::Project> m_Project;
	BitPounce::OrthographicCamera m_Camera;
	bool OnAssetPreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnScenePreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnWindowResize(BitPounce::WindowResizeEvent& e);

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
