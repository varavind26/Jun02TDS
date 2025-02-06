// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "JunTDS/FuncLibrary/Types.h"
#include "Engine/DataTable.h"
#include "EnemyDefault.generated.h"

UCLASS()
class JUNTDS_API AEnemyDefault : public AActor
{
	GENERATED_BODY()

public:
	AEnemyDefault();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		UDataTable* DamageMultiplierTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
		class USkeletalMeshComponent* EnemySkeletalMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
		float MaxHealth = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
		float MaxArmor = 100.0f;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
		float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Armor")
		float CurrentArmor;

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void EnemyTakeAnyDamage(const FHitResult& HitResult, float BaseDamage, bool electric, bool chemical);
	//UFUNCTION(BlueprintCallable)
		float CalculateDamage(const FHitResult& HitResult, float BaseDamage);
};
