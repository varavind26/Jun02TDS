#include "EnemyDefault.h"

AEnemyDefault::AEnemyDefault()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    RootComponent = SceneComponent;

    EnemySkeletalMesh = CreateAbstractDefaultSubobject<USkeletalMeshComponent>(TEXT("Enemy Skeltal Mesh"));
    EnemySkeletalMesh->SetupAttachment(RootComponent);
    EnemySkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemyDefault::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    CurrentArmor = MaxArmor;
}

void AEnemyDefault::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnemyDefault::EnemyTakeAnyDamage(const FHitResult& HitResult, float BaseDamage, bool electric, bool chemical)
{
    if (CurrentHealth <= 0.0f)
    {
        return;
    }
    if (electric)
    {
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FName BoneName = HitResult.BoneName;
        for (int i = 0; i < 3; ++i)
        {
            CurrentHealth -= (Damage/4);
            UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Armor: %f"), *BoneName.ToString(), CurrentHealth, CurrentArmor);
        }
        if (CurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
            Destroy();
        }
    }
    else if (chemical)
    {
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FName BoneName = HitResult.BoneName;

        FBodyPartDamageMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageMultiplier>(BoneName, TEXT("Body Part Damage"));
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
                Destroy();
            }
        }
        else
        {
            if (MultiplierRow && CurrentArmor > 0.0f)
            {
                for (int i = 0; i < 5; ++i)
                {
                    CurrentArmor -= (MultiplierRow->ArmorMultiplier)/3;
                    CurrentArmor = FMath::Max(CurrentArmor, 0.0f);
                    UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Damage: %f, Damage:%f, Armor: %f"), *BoneName.ToString(), CurrentHealth, Damage, CurrentArmor);
                }
            }
            if (CurrentHealth <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
                Destroy();
            }
        }
    }
    else
    {
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FName BoneName = HitResult.BoneName;

        FBodyPartDamageMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageMultiplier>(BoneName, TEXT("Body Part Damage"));
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

            Destroy();
        }
    }

}

float AEnemyDefault::CalculateDamage(const FHitResult& HitResult, float BaseDamage)
{
    if (!HitResult.GetComponent() || !HitResult.BoneName.IsValid())
        return BaseDamage;

    FBodyPartDamageMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageMultiplier>(HitResult.BoneName, TEXT("Body Part Damage"));

    float DamageMultiplier = MultiplierRow->DamageMultiplier;
    float ArmorMultiplier = MultiplierRow->ArmorMultiplier;
  
    if (CurrentArmor <= 0)
    {
        float FinalDamage = BaseDamage * DamageMultiplier;
        return FMath::Max(FinalDamage, 0.0f);
    }
    else
    {
        float FinalDamage = (BaseDamage * DamageMultiplier) - ArmorMultiplier;;
        return FMath::Max(FinalDamage, 0.0f);
    }
}
