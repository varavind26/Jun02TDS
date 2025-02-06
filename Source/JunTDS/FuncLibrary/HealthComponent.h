#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JUNTDS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    UFUNCTION(BlueprintCallable)
        void ApplyDamage(float Damage);

    UFUNCTION(BlueprintPure)
        float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure)
        bool IsDead() const { return CurrentHealth <= 0; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Health")
        float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        float CurrentHealth;

    virtual void BeginPlay() override;
};