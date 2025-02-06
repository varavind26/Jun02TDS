// Fill out your copyright notice in the Description page of Project Settings.


#include "ShellProjectileDefault.h"

// Sets default values
AShellProjectileDefault::AShellProjectileDefault()
{
    // Включить вызов функции Tick() каждый кадр.
    PrimaryActorTick.bCanEverTick = true;

    // Создать компонент сферы столкновения.
    ShellCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

    // Установить радиус сферы столкновения.
    ShellCollisionSphere->SetSphereRadius(16.f);

    // Настроить поведение при столкновении.
    ShellCollisionSphere->bReturnMaterialOnMove = true;

    // Отключить влияние на навигацию.
    ShellCollisionSphere->SetCanEverAffectNavigation(false);

    // Сделать сферу столкновения корневым компонентом.
    RootComponent = ShellCollisionSphere;

    // Создать компонент статической сетки снаряда.
    ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell Projectile Mesh"));
    ShellMesh->SetupAttachment(RootComponent);
    ShellMesh->SetCanEverAffectNavigation(false);

    // Создать компонент звука снаряда (закомментирован).
    //ShellSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Shell Audio"));
    //ShellSound->SetupAttachment(RootComponent);

    // Создать компонент движения снаряда.
    ShellProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Shell ProjectileMovement"));
    ShellProjectileMovement->UpdatedComponent = RootComponent;
    ShellProjectileMovement->InitialSpeed = 1.f;
    ShellProjectileMovement->MaxSpeed = 0.f;

    // Настроить движение снаряда.
    ShellProjectileMovement->bRotationFollowsVelocity = true;
    ShellProjectileMovement->bShouldBounce = true;
}

// Called when the game starts or when spawned
void AShellProjectileDefault::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AShellProjectileDefault::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

