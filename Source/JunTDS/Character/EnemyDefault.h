// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "JunTDS/FuncLibrary/HealthComponent.h"
#include "Engine.h"
#include "Engine/DataTable.h"
#include "EnemyDefault.generated.h"

UCLASS()
class JUNTDS_API AEnemyDefault : public AActor
{
	GENERATED_BODY()

public:
	AEnemyDefault();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USkeletalMeshComponent* EnemySkeletalMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		float CharacterCurrentHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		float CharacterMaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		float CharacterCurrentArmor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		float CharacterMaxArmor = 100.f;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	void InitializeHealth();
	void InitializeArmor();
	void Dead();
	void GetCurrentHealth();
};
