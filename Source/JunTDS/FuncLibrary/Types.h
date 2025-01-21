// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types.generated.h"



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
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float AimWalkSpeed = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float WalkSpeed = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float RunSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float SprintRunSpeed = 800.0f;
};

UCLASS()
class JUNTDS_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};


