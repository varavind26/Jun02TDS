// Copyright Epic Games, Inc. All Rights Reserved.

#include "JunTDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

AJunTDSCharacter::AJunTDSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/Blueprints/Character/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AJunTDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void AJunTDSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, & AJunTDSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, & AJunTDSCharacter::InputAxisY);
}

void AJunTDSCharacter::InputAxisX(float Value)
{
	AxisX = Value;
}

void AJunTDSCharacter::InputAxisY(float Value)
{
	AxisY = Value;
}

void AJunTDSCharacter::MovementTick(float DeltaTime)
{
	
	APlayerController* myController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (myController)
	{
		// Разваричваем персонажа в сторону курсора
		FHitResult ResultHit;
		myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

		//AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
		//AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

		// Персонаж движется вперед в направлении курсора
		FVector ForwardDirection = (ResultHit.Location - GetActorLocation()).GetSafeNormal();
		ForwardDirection.Normalize();
		AddMovementInput(ForwardDirection, AxisX);
		
		// Персонаж движется влево-вправо относительно курсора
		if (SprintRunEnable)
		{
			AddMovementInput(FVector(0.0f,0.0f,0.0f), AxisY);
		}
		else
		{
			FVector Strafe = UKismetMathLibrary::RotateAngleAxis(ForwardDirection, 90.f, FVector::UpVector);
			AddMovementInput(Strafe, AxisY);
		}
	}
}

void AJunTDSCharacter::CharacterUpdate()
{
	float ResSpeed = 600.0f;
	switch (MovementState)
	{
	case EMovementState::Aim_State:
		ResSpeed = MovementSpeedInfo.AimSpeed;
		break;
	case EMovementState::AimWalk_State:
		ResSpeed = MovementSpeedInfo.AimWalkSpeed;
		break;
	case EMovementState::Walk_State:
		ResSpeed = MovementSpeedInfo.WalkSpeed;
		break;
	case EMovementState::Run_State:
		ResSpeed = MovementSpeedInfo.RunSpeed;
		break;
	case EMovementState::SprintRun_State:
		ResSpeed = MovementSpeedInfo.SprintRunSpeed;
		break;
	default:
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void AJunTDSCharacter::ChangeMovementState()
{
	FVector LastMovementInputVector = GetLastMovementInputVector();
	bool IsMovingForward = FVector::DotProduct(LastMovementInputVector, GetActorForwardVector()) > 0;
	
	if (SprintRunEnable && !IsMovingForward)
	{
		SprintRunEnable = false;
	}

	if (!WalkEnable && !SprintRunEnable && !AimEnable)
	{
		MovementState = EMovementState::Run_State;
	}
	else
	{
		if (SprintRunEnable && IsMovingForward)
		{
			WalkEnable = false;
			AimEnable = false;
			MovementState = EMovementState::SprintRun_State;
		}
		else
		{
			if (WalkEnable && !SprintRunEnable && AimEnable)
			{
				MovementState = EMovementState::AimWalk_State;
			}
			else
			{
				if (WalkEnable && !SprintRunEnable && !AimEnable)
				{
					MovementState = EMovementState::Walk_State;
				}
				else
				{
					if (!WalkEnable && !SprintRunEnable && AimEnable)
					{
						MovementState = EMovementState::Aim_State;
					}
				}
			}
		}
	}

	CharacterUpdate();
}

