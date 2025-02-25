#include "ShellProjectileDefault.h"

AShellProjectileDefault::AShellProjectileDefault()
{
    PrimaryActorTick.bCanEverTick = true;

    ShellCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

    ShellCollisionSphere->SetSphereRadius(16.f);

   
    ShellCollisionSphere->bReturnMaterialOnMove = true;

    ShellCollisionSphere->SetCanEverAffectNavigation(false);

    RootComponent = ShellCollisionSphere;

    ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell Projectile Mesh"));
    ShellMesh->SetupAttachment(RootComponent);
    ShellMesh->SetCanEverAffectNavigation(false);

    //ShellSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Shell Audio"));
    //ShellSound->SetupAttachment(RootComponent);

    ShellProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Shell ProjectileMovement"));
    ShellProjectileMovement->UpdatedComponent = RootComponent;
    ShellProjectileMovement->InitialSpeed = 1.f;
    ShellProjectileMovement->MaxSpeed = 0.f;

    ShellProjectileMovement->bRotationFollowsVelocity = true;
    ShellProjectileMovement->bShouldBounce = true;
}

void AShellProjectileDefault::BeginPlay()
{
    Super::BeginPlay();

}

void AShellProjectileDefault::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

