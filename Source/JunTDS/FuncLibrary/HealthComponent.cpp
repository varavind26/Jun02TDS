#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(float Damage)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
}