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
	virtual void OnImGuiRender() override;
	void OnEvent(BitPounce::Event& e) override;

	
private:
    BitPounce::Ref<BitPounce::Scene> m_Scene;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
