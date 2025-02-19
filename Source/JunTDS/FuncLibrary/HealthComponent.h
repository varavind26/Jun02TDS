#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

USTRUCT(BlueprintType)
struct FBodyPartDamageHealthMultiplier : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FName BoneName;
  
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DamageMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float CoefArmorMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ArmorMultiplier = 1.0f;
 };

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JUNTDS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UHealthComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        float CurrentArmor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
        UDataTable* DamageMultiplierTable;

    UFUNCTION(BlueprintCallable)
        void ApplyDamage(float Damage);

    UFUNCTION(BlueprintCallable)
        void SetInitialHealth(float InitialHealth);
    UFUNCTION(BlueprintCallable)
        void SetInitialArmor(float InitialArmor);

    UFUNCTION(BlueprintPure)
        float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure)
        bool IsDead() const { return CurrentHealth <= 0; }

    UFUNCTION(BlueprintCallable)
        void TakeAnyDamage(const FHitResult& HitResult, float BaseDamage, bool electric, bool chemical);
    UFUNCTION(BlueprintCallable)
        float CalculateDamage(const FHitResult& HitResult, float BaseDamage);

protected:
   
    virtual void BeginPlay() override;
};