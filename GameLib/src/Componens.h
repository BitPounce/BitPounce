#pragma once

#include <BitPounce.h>

template<typename T, T min>
struct HealthSystem
{
    T Health;
    T MaxHealth;
    BitPounce::FunctionArray<void(T newHealth, T MaxHealth, T oldHealth, HealthSystem<T, min> healthSystem)> OnDied {};
    BitPounce::FunctionArray<void(T newHealth, T MaxHealth, T oldHealth, HealthSystem<T, min> healthSystem)> OnHealed {};
    BitPounce::FunctionArray<void(T newHealth, T MaxHealth, T oldHealth, HealthSystem<T, min> healthSystem)> OnDamged {};
    BitPounce::FunctionArray<void(T newHealth, T MaxHealth, T oldHealth, HealthSystem<T, min> healthSystem)> OnHealthChanged {};

    HealthSystem() = delete;
    HealthSystem(T health)
    {
        MaxHealth = health;
        this->Health = MaxHealth;
    }

    T GetHealthPercent()
    {
        return (T)Health / MaxHealth;
    }

    void Damage(T damageAmount)
    {
        T oldHealth = Health;
        Health -= damageAmount;
        if(Health < min) {Health = min; OnDied(min, MaxHealth, oldHealth, *this);}
        OnHealthChanged(Health, MaxHealth, oldHealth, *this);
    }

    void Heal(T HealAmount)
    {
        T oldHealth = Health;
        Health += HealAmount;
        if(Health > MaxHealth) {Health = MaxHealth;}
        OnHealthChanged(Health, MaxHealth, oldHealth, *this);
    }
};



struct Player
{
    BitPounce::Entity tilemap;
    uint8_t frime = 0;
    uint8_t fps = 4;
    BitPounce::Timer Timer;
    bool HasMoved = false;
    BitPounce::Timer Timer2;
    std::vector<BitPounce::Ref<BitPounce::Audio>> Audios;
    std::function<void(uint32_t seed)> onWin;
    std::function<void()> onKilledEnemy;
};

struct Enemy
{
    BitPounce::Entity Player;
    uint8_t frime = 0;
    uint8_t fps = 4;
    BitPounce::Timer Timer;
    float speed = 5.f;
    uint8_t yPos = 0;
    BitPounce::Timer Timer2;
};

struct Window
{
    uint32_t seed;
    float yPos;
    float timer;
};