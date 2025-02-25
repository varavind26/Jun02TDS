// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JunTDS/FuncLibrary/Types.h"
#include "JunTDS/WeaponDefault.h"
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

	float AxisX = 0.0f;
	float AxisY = 0.0f;

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
	
	UFUNCTION()
		void OnAimPressed();
	UFUNCTION()
		void OnAimReleased();
	
	UFUNCTION(BlueprintCallable)
		void AttackCharEvent(bool bIsFiring);

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

