#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHealthComponent::ApplyDamage(float Damage)
{
}

void UHealthComponent::SetInitialHealth(float InitialHealth)
{
    CurrentHealth = InitialHealth;
}

void UHealthComponent::SetInitialArmor(float InitialArmor)
{
    CurrentArmor = InitialArmor;
}

void UHealthComponent::TakeAnyDamage(const FHitResult& HitResult, float BaseDamage, bool electric, bool chemical)
{
    if (CurrentHealth <= 0.0f)
    {
        return;
    }
    if (electric)
    {      
        FName BoneName = HitResult.BoneName;
        float Damage = CalculateDamage(HitResult, BaseDamage);
        for (int i = 0; i < 3; ++i)
        {
            CurrentHealth -= (Damage / 4);
            UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Armor: %f"), *BoneName.ToString(), CurrentHealth, CurrentArmor);
        }
        if (CurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
        }
    }
    else if (chemical)
    {  
        FName BoneName = HitResult.BoneName;
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(BoneName, TEXT("Body Part Damage"));
        CurrentHealth -= Damage;

        if (CurrentArmor <= 0)
        {

            for (int i = 0; i < 5; ++i)
            {
                CurrentHealth -= (Damage / 5);
                UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Damage:%f, Armor: %f"), *BoneName.ToString(), CurrentHealth, Damage, CurrentArmor);
            }
            if (CurrentHealth <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
            }
        }
        else
        {
            if (MultiplierRow && CurrentArmor > 0.0f)
            {
                for (int i = 0; i < 5; ++i)
                {
                    CurrentArmor -= (MultiplierRow->ArmorMultiplier) / 3;
                    CurrentArmor = FMath::Max(CurrentArmor, 0.0f);
                    UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Damage: %f, Damage:%f, Armor: %f"), *BoneName.ToString(), CurrentHealth, Damage, CurrentArmor);
                }
            }
            if (CurrentHealth <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
            }
        }
    }
    else
    {
        FName BoneName = HitResult.BoneName;
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(BoneName, TEXT("Body Part Damage"));
        CurrentHealth -= Damage;
        if (MultiplierRow && CurrentArmor > 0.0f)
        {
            CurrentArmor -= MultiplierRow->ArmorMultiplier;
            CurrentArmor = FMath::Max(CurrentArmor, 0.0f);
        }

        UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Armor: %f"), *BoneName.ToString(), CurrentHealth, CurrentArmor);

        if (CurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
        }
    }
}

float UHealthComponent::CalculateDamage(const FHitResult& HitResult, float BaseDamage)
{
    if (!HitResult.GetComponent() || !HitResult.BoneName.IsValid())
        return BaseDamage;

    FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(HitResult.BoneName, TEXT("Body Part Damage"));

    float DamageMultiplier = MultiplierRow->DamageMultiplier;
    float ArmorMultiplier = MultiplierRow->ArmorMultiplier;
    float CoefArmorMultiplier = MultiplierRow->CoefArmorMultiplier;

    if (CurrentArmor <= 0)
    {
        float FinalDamage = BaseDamage * DamageMultiplier * CoefArmorMultiplier;
        return FMath::Max(FinalDamage, 0.0f);
    }
    else
    {
        float FinalDamage = ((BaseDamage * DamageMultiplier) - ArmorMultiplier) * CoefArmorMultiplier;;
        return FMath::Max(FinalDamage, 0.0f);
    }
}
