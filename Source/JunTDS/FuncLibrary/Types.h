// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Types.generated.h"

USTRUCT(BlueprintType)
struct FBodyPartDamageMultiplier : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ArmorMultiplier = 1.0f;
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Aim_State UMETA(DispalyName = "Aim State"),
	AimWalk_State UMETA(DispalyName = "Aim Walk State"),
	Walk_State UMETA(DispalyName = "Walk State"),
	Run_State UMETA(DispalyName = "Run State"),
	SprintRun_State UMETA(DispalyName = "Sprint Run State"),
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Aim_StateDispersionAimMax = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Aim_StateDispersionAimMin = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Aim_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Aim_StateDispersionReduction = .3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float AimWalk_StateDispersionAimMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float AimWalk_StateDispersionAimMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float AimWalk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float AimWalk_StateDispersionReduction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Walk_StateDispersionAimMax = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Walk_StateDispersionAimMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Walk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Walk_StateDispersionReduction = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Run_StateDispersionAimMax = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Run_StateDispersionAimMin = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Run_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		float Run_StateDispersionReduction = 0.1f;
};

USTRUCT(BlueprintType)
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimSpeed = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimWalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float WalkSpeed = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float RunSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float SprintRunSpeed = 900.0f;
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		TSubclassOf<class AProjectileDefault> Projectile = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileLifeTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileInitSpeed = 2000.0f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		//TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface> HitDecals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		USoundBase* HitSound = nullptr;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		//TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> HitFXs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		UParticleSystem* ExploseFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		USoundBase* ExploseSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ProjectileMaxRadiusDamage = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		float ExploseMaxDamage = 40.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		bool bIsElectric = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
		bool bIsChemical = false;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
		TSubclassOf<class AWeaponDefault> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float RateOfFire = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		float ReloadTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		int32 MaxRound = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		int32 NumberProjectileByShot = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		FProjectileInfo ProjectileSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		USoundBase* SoundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
		USoundBase* SoundReloadWeapon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* AnimCharFire = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation ")
		UAnimMontage* AnimCharReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX ")
		UParticleSystem* EffectFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
		float WeaponDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
		float DistacneTrace = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMesh* MagazineDrop = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
		UStaticMesh* ShellBullets = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect")
		UDecalComponent* DecalOnHit = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
		FWeaponDispersion DispersionWeapon;
};

USTRUCT(BlueprintType)
struct FAddicionalWeaponInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
		int32 Round = 10;
};

UCLASS()
class JUNTDS_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};


