// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "FuncLibrary/Types.h"
#include "ShellProjectileDefault.generated.h"

UCLASS()
class JUNTDS_API AShellProjectileDefault : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShellProjectileDefault();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UStaticMeshComponent* ShellMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USphereComponent* ShellCollisionSphere = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UProjectileMovementComponent* ShellProjectileMovement = nullptr;

	FProjectileInfo ProjectileSetting;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
