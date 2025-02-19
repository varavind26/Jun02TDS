#include "EnemyDefault.h"

AEnemyDefault::AEnemyDefault()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    RootComponent = SceneComponent;

    EnemySkeletalMesh = CreateAbstractDefaultSubobject<USkeletalMeshComponent>(TEXT("Enemy Skeltal Mesh"));
    EnemySkeletalMesh->SetupAttachment(RootComponent);
    EnemySkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
 }

void AEnemyDefault::BeginPlay()
{
    Super::BeginPlay();

    InitializeHealth();
    InitializeArmor();
    GetCurrentHealth();
}

void AEnemyDefault::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (HealthComponent->IsDead())
    {
        Dead();
    }
}

void AEnemyDefault::InitializeHealth()
{
    HealthComponent->SetInitialHealth(CharacterMaxHealth);
    CharacterCurrentHealth = HealthComponent->CurrentHealth;
}

void AEnemyDefault::InitializeArmor()
{
    HealthComponent->SetInitialArmor(CharacterMaxArmor);
    CharacterCurrentArmor = HealthComponent->CurrentArmor;
}

void AEnemyDefault::Dead()
{
    Destroy();
}

void AEnemyDefault::GetCurrentHealth()
{
    float HP = HealthComponent->GetCurrentHealth();
    UE_LOG(LogTemp, Warning, TEXT("HP: %f"), HealthComponent);
}

/*void AEnemyDefault::EnemyTakeAnyDamage(const FHitResult& HitResult, float BaseDamage, bool electric, bool chemical)
{
    if (CharacterCurrentHealth <= 0.0f)
    {
        return;
    }
    if (electric)
    {
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FName BoneName = HitResult.BoneName;
        for (int i = 0; i < 3; ++i)
        {
            CharacterCurrentHealth -= (Damage/4);
            UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Armor: %f"), *BoneName.ToString(), CharacterCurrentHealth, CurrentArmor);
        }
        if (CharacterCurrentHealth <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enemy died!"));
            Destroy();
        }
    }
    else if (chemical)
    {
        float Damage = CalculateDamage(HitResult, BaseDamage);
        FName BoneName = HitResult.BoneName;

        FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(BoneName, TEXT("Body Part Damage"));
        CharacterCurrentHealth -= Damage;
        
        if (CurrentArmor <= 0)
        {
            
            for (int i = 0; i < 5; ++i)
            {
                CharacterCurrentHealth -= (Damage / 5);
                UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Damage:%f, Armor: %f"), *BoneName.ToString(), CharacterCurrentHealth, Damage, CurrentArmor);
            }
            if (CharacterCurrentHealth <= 0.0f)
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
                    UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Damage: %f, Damage:%f, Armor: %f"), *BoneName.ToString(), CharacterCurrentHealth, Damage, CurrentArmor);
                }
            }
            if (CharacterCurrentHealth <= 0.0f)
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

        FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(BoneName, TEXT("Body Part Damage"));
        CharacterCurrentHealth -= Damage;
        if (MultiplierRow && CurrentArmor > 0.0f)
        {
            CurrentArmor -= MultiplierRow->ArmorMultiplier;
            CurrentArmor = FMath::Max(CurrentArmor, 0.0f);
        }

        UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s, Health: %f, Armor: %f"), *BoneName.ToString(), CharacterCurrentHealth, CurrentArmor);

        if (CharacterCurrentHealth <= 0.0f)
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

    FBodyPartDamageHealthMultiplier* MultiplierRow = DamageMultiplierTable->FindRow<FBodyPartDamageHealthMultiplier>(HitResult.BoneName, TEXT("Body Part Damage"));

    float DamageMultiplier = MultiplierRow->DamageMultiplier;
    float ArmorMultiplier = MultiplierRow->ArmorMultiplier;
    float CoefArmorMultiplier = MultiplierRow->CoefArmorMultiplier;
  
    if (CurrentArmor <= 0)
    {
        float FinalDamage = BaseDamage * DamageMultiplier* CoefArmorMultiplier;
        return FMath::Max(FinalDamage, 0.0f);
    }
    else
    {
        float FinalDamage = ((BaseDamage * DamageMultiplier) - ArmorMultiplier)* CoefArmorMultiplier;;
        return FMath::Max(FinalDamage, 0.0f);
    }
}*/
