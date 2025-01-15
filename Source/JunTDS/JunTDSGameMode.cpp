// Copyright Epic Games, Inc. All Rights Reserved.

#include "JunTDSGameMode.h"
#include "JunTDSPlayerController.h"
#include "JunTDSCharacter.h"
#include "UObject/ConstructorHelpers.h"

AJunTDSGameMode::AJunTDSGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AJunTDSPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}