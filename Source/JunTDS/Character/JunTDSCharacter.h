// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JunTDS/FuncLibrary/Types.h"
#include "JunTDS/WeaponDefault.h"
#include <cmath>
#include "JunTDSCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSpeedInfo
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class AJunTDSCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AJunTDSCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:


	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;

public:
	// MOVEMENT
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		EMovementState MovementState = EMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FCharacterSpeed MovementSpeedInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool WalkEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool AimEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool RunEnable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool SprintRunEnable = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector CursorLocation;

	UFUNCTION()
		void InputAxisX(float value);
	UFUNCTION()
		void InputAxisY(float value);

	UFUNCTION()
		void LookVertical(float value);
	UFUNCTION()
		void LookHorizontal(float value);

	float AxisX = 0.0f;
	float AxisY = 0.0f;

	// Climb Props
	UPROPERTY(BlueprintReadOnly)
		bool bIsClimb = false;
	UPROPERTY(BlueprintReadOnly)
		bool bIsMantle = false;
	UPROPERTY(BlueprintReadOnly)
		bool bIsHurdle = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float DistToObject = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float DistanceToHeightTrace = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float WallOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MinHeightToClimb = -110.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MaxHeightToClimb = -190.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float ClimbHandHeight = 158.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MinHeightToMantle = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MaxHeightToMantle = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MantleHandHeight = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MinHeightToHurdle = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float MaxHeightToHurdle = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		float HurdleHandHeight = 100.f;
	

	UPROPERTY(BlueprintReadOnly)
		FVector WallHitLocationToClimb;
	UPROPERTY(BlueprintReadOnly)
		FVector WallHitNormalToClimb;
	UPROPERTY(BlueprintReadOnly)
		FVector HeightHitLocationToClimb;
	UPROPERTY(BlueprintReadOnly)
		FVector HeightHitNormalToClimb;

	//Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		UAnimMontage* ClimbingAnimation = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		UAnimMontage* MantleAnimation = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
		UAnimMontage* HurdleAnimation = nullptr;



	//Weapon	
	AWeaponDefault* CurrentWeapon = nullptr;

	//for demo 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
		FName InitWeaponName;

	UFUNCTION(BlueprintCallable)
		AWeaponDefault* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable)
		void InitWeapon(FName IdWeaponName);
	UFUNCTION(BlueprintCallable)
		void TryReloadWeapon();
	UFUNCTION()
		void WeaponReloadStart(UAnimMontage* Anim);
	UFUNCTION()
		void WeaponReloadEnd();

	// Ation Events
	UFUNCTION()
		void InputAttackPressed();
	UFUNCTION()
		void InputAttackReleased();

	UFUNCTION()
		void OnSprintPressed();
	UFUNCTION()
		void OnSprintReleased();

	//climb
	UFUNCTION(BlueprintCallable)
		bool Traversal();
	UFUNCTION(BlueprintCallable)
		void Climbing();
	UFUNCTION(BlueprintCallable)
		void OnClimbFinished(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
		void OnAimPressed();
	UFUNCTION()
		void OnAimReleased();
	
	UFUNCTION(BlueprintCallable)
		void AttackCharEvent(bool bIsFiring);

	bool FindIntersectionPoint(const FVector& A1, const FVector& A2, const FVector& B1, const FVector& B2, FVector& OutIntersectionPoint);
	// Tick Func

	UFUNCTION(BlueprintCallable)
		void CharacterUpdate();

	UFUNCTION(BlueprintCallable)
		void ChangeMovementState();

	UFUNCTION()
		void Movement();

	UFUNCTION()
		void UpdateStateWeapon(EMovementState NewMovementState);

	UFUNCTION()
		void ChangeDispersion();
};

