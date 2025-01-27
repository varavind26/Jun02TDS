// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JunTDS/FuncLibrary/Types.h"
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

public:
	AJunTDSCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Movement")
		EMovementState MovementState = EMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FCharacterSpeed MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool SprintRunEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool WalkEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool AimEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool RunEnable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FVector StrafeVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector CursorLocation;

	UFUNCTION()
		void InputAxisX(float value);
	UFUNCTION()
		void InputAxisY(float value);

	float AxisX = 0.0f;
	float AxisY = 0.0f;
	// Tick Func

	UFUNCTION()
		void MovementTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
		void CharacterUpdate();

	UFUNCTION(BlueprintCallable)
		void ChangeMovementState();
};

