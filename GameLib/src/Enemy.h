#pragma once
#include <BitPounce.h>
#include "Componens.h"

class EnemySystem : public BitPounce::ECSSystem
{

public:
    virtual BitPounce::System* clone() const override
	{
		return new EnemySystem(*this);
	}
    virtual void OnUpdate(BitPounce::Timestep& ts) override;
    EnemySystem(/* args */);
    ~EnemySystem();
private:
    /* data */
};

