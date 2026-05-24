#pragma once
#include <BitPounce.h>

class Test3D: public BitPounce::Layer
{

public:
	Test3D(BitPounce::UILayer* ui);
	virtual ~Test3D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(BitPounce::Timestep& ts) override;
	virtual void OnImGuiRender(BitPounce::Timestep& ts) override;
	void OnEvent(BitPounce::Event& e) override;

	
private:
	BitPounce::Ref<BitPounce::Project> m_Project;
    BitPounce::EditorCamera m_Camera;
	bool OnAssetPreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnScenePreloaded(BitPounce::AssetPreLoadedEvent& e);
	bool OnWindowResize(BitPounce::WindowResizeEvent& e);
	BitPounce::UILayer* m_UILayer;
    BitPounce::Ref<BitPounce::Shader> m_Shader;
    BitPounce::Ref<BitPounce::Texture2D> m_Tex;
    BitPounce::Ref<BitPounce::Mesh> m_Mesh;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
