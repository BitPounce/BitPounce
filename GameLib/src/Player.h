#pragma once
#include <BitPounce.h>
#include "Componens.h"

class PlayerSystem : public BitPounce::ECSSystem
{

public:
    virtual BitPounce::System* clone() const override
	{
		return new PlayerSystem(*this);
	}
    virtual void OnUpdate(BitPounce::Timestep& ts) override;
    virtual void OnImGuiDraw(BitPounce::Timestep& ts) override;
    virtual void OnRuntimeStart() override;
    PlayerSystem(/* args */);
    ~PlayerSystem();
private:
    
};

