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