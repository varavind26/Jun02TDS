// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JunTDSPlayerController.generated.h"

UCLASS()
class AJunTDSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AJunTDSPlayerController();

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	/*uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	
	void OnResetVR();

	void MoveToMouseCursor();

	
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	void SetNewMoveDestination(const FVector DestLocation);

	void OnSetDestinationPressed();
	void OnSetDestinationReleased();*/
};


