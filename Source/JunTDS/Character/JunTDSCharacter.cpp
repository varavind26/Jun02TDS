// Copyright Epic Games, Inc. All Rights Reserved.

#include "JunTDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "JunTDS/Game/JunTDSGameInstance.h"

AJunTDSCharacter::AJunTDSCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	PrimaryActorTick.bCanEverTick = true;
}



void AJunTDSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AJunTDSCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitWeapon(InitWeaponName);
}

void AJunTDSCharacter::InputAxisX(float Value)
{
	if (SprintRunEnable && Value <= 0.0f)
	{
		OnSprintReleased();
	}

	AxisX = SprintRunEnable ? FMath::Clamp(Value, 0.0f, 1.0f) : Value;
	Movement();
}

void AJunTDSCharacter::InputAxisY(float Value)
{
	AxisY = SprintRunEnable ? 0.0f : Value;
	Movement();
}

void AJunTDSCharacter::Movement()
{
	if (!bIsClimb)
	{
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	
		AddMovementInput(ForwardDirection, AxisX);
		AddMovementInput(RightDirection, AxisY);
		if (CurrentWeapon)
		{
			FVector CameraLoc;
			FRotator CameraRot;
			GetController()->GetPlayerViewPoint(CameraLoc, CameraRot);

			const FVector TraceEnd = CameraLoc + (CameraRot.Vector() * 10000.0f);
			CurrentWeapon->ShootEndLocation = TraceEnd;
		}
	}
}

void AJunTDSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, &AJunTDSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, &AJunTDSCharacter::InputAxisY);
	NewInputComponent->BindAxis(TEXT("LookHorizontal"), this, &APawn::AddControllerYawInput);
	NewInputComponent->BindAxis(TEXT("LookVertical"), this, &APawn::AddControllerPitchInput);

	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::InputAttackReleased);
	NewInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::OnSprintPressed);
	NewInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &AJunTDSCharacter::OnSprintReleased);
	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::OnAimPressed);
	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::OnAimReleased);
	NewInputComponent->BindAction(TEXT("Climb"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::Climbing);
	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::TryReloadWeapon);
}

void AJunTDSCharacter::InitWeapon(FName IdWeaponName)
{
	UJunTDSGameInstance* myGI = Cast<UJunTDSGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;

	if (myGI && myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo) && myWeaponInfo.WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		if (AWeaponDefault* myWeapon = GetWorld()->SpawnActor<AWeaponDefault>(myWeaponInfo.WeaponClass, SpawnParams))
		{
			CurrentWeapon = myWeapon;

			FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
			myWeapon->AttachToComponent(GetMesh(), Rule, FName("hand_r_weaponSocket"));
			CurrentWeapon = myWeapon;

			myWeapon->WeaponSetting = myWeaponInfo;
			myWeapon->WeaponInfo.Round = myWeaponInfo.MaxRound;
			myWeapon->ReloadTime = myWeaponInfo.ReloadTime;
			myWeapon->UpdateStateWeapon(MovementState);

			myWeapon->OnWeaponReloadStart.AddDynamic(this, &AJunTDSCharacter::WeaponReloadStart);
			myWeapon->OnWeaponReloadEnd.AddDynamic(this, &AJunTDSCharacter::WeaponReloadEnd);
		}
	}
}


void AJunTDSCharacter::OnAimPressed()
{
	AimEnable = true;
	ChangeMovementState();
}

void AJunTDSCharacter::OnAimReleased()
{
	AimEnable = false;
	ChangeMovementState();
}

void AJunTDSCharacter::Climbing()
{
	if (ClimbingAnimation != nullptr && TraceToClimb())
	{
		bIsClimb = true;
		MovementState = EMovementState::Climb_State;
		FRotator LastActorRotator = GetActorRotation();
		FVector LastActorLocation = GetActorLocation();

		GetCapsuleComponent()->SetRelativeLocation(FVector(WallHitLocationToClimb.X, WallHitLocationToClimb.Y, (HeightHitLocationToClimb.Z - HandUpperPostitionToClimb)));
		GetCapsuleComponent()->SetRelativeRotation(LastActorRotator);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->Montage_Play(ClimbingAnimation, 1.0f);
	}
}

void AJunTDSCharacter::OnClimbFinished(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("Climbing animation finished!"));
	FRotator LastActorRotator = GetActorRotation();
	GetCapsuleComponent()->SetRelativeLocation(FVector(HeightHitLocationToClimb.X, HeightHitLocationToClimb.Y, (HeightHitLocationToClimb.Z+96.225031)));
	bIsClimb = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->AdjustFloorHeight(); //FVector(HeightHitLocationToClimb.X, HeightHitLocationToClimb.Y, HeightHitLocationToClimb.Z));
}

void AJunTDSCharacter::OnSprintPressed()
{
	if (AxisX > 0.1f)
	{
		SprintRunEnable = true;
		AttackCharEvent(false);
		ChangeMovementState();
	}
}

void AJunTDSCharacter::OnSprintReleased()
{
	SprintRunEnable = false;
}

bool AJunTDSCharacter::TraceToClimb()
{
	FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * DistToObject);
	FHitResult HitResult;

	FVector TraceSize = FVector(0.f, 0.f, 500.f);
	FVector HeighTraceStart = GetActorLocation() + TraceSize + GetActorForwardVector() * DistanceToHeightTrace;
	FVector HeighTraceEnd = HeighTraceStart - TraceSize;
	FHitResult HeighTraceHitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	FVector PelvisLocation = GetMesh()->GetSocketLocation(TEXT("pelvisSocket"));

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	WallHitLocationToClimb = HitResult.Location;
	WallHitNormalToClimb = HitResult.Normal;
	bool bHeighTraceHit = GetWorld()->LineTraceSingleByChannel(HeighTraceHitResult, HeighTraceStart, HeighTraceEnd, ECC_Visibility, QueryParams);
	HeightHitLocationToClimb = HeighTraceHitResult.Location;
	float ToClimbValue = (HeightHitLocationToClimb.Z- PelvisLocation.Z);
	bool CheckClimb = ((ToClimbValue>=MinHeightToClimb) && (ToClimbValue <= MaxHeightToClimb));
	if (bHeighTraceHit && bHit && CheckClimb && !bIsClimb)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, (uint8)'\000', 2.f);
		DrawDebugLine(GetWorld(), HeighTraceStart, HeighTraceEnd, FColor::Red, false, 5.f, (uint8)'\000', 2.f);
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		GetCharacterMovement()->StopMovementImmediately();
		return true;
	}
	else
	{
		return false;
	}
}

void AJunTDSCharacter::InputAttackPressed()
{
	AttackCharEvent(true);
	if (CurrentWeapon != nullptr && !CurrentWeapon->WeaponReloading)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponSetting.AnimCharFire, 1.0f);
		if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentWeapon->WeaponSetting.AnimCharFire))
		{
			GetMesh()->GetAnimInstance()->StopAllMontages(0.1f);
			GetMesh()->GetAnimInstance()->Montage_Play(CurrentWeapon->WeaponSetting.AnimCharFire, 1.0f);
		}
	}
	else
	{
		if (CurrentWeapon != nullptr)
		{
			if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentWeapon->WeaponSetting.AnimCharFire))
			{
				GetMesh()->GetAnimInstance()->StopAllMontages(0.1f);
			}
		}
	}
}

void AJunTDSCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
	if (CurrentWeapon != nullptr && !CurrentWeapon->WeaponReloading)
	{
		if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentWeapon->WeaponSetting.AnimCharFire))
		{
			GetMesh()->GetAnimInstance()->StopAllMontages(0.1f);
		}
	}
	else
	{
		if (CurrentWeapon != nullptr)
		{
			if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentWeapon->WeaponSetting.AnimCharFire))
			{
				GetMesh()->GetAnimInstance()->StopAllMontages(0.1f);
			}
		}
	}
}

void AJunTDSCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (SprintRunEnable)
	{
		myWeapon->SetWeaponStateFire(false);
		return;
	}

	myWeapon->SetWeaponStateFire(bIsFiring);
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
	AttackCharEvent(false);
	if (bIsClimb)
	{
		WalkEnable = false;
		AimEnable = false;
		SprintRunEnable = false;
		MovementState = EMovementState::Climb_State;
		AttackCharEvent(false);
	}
	else
	{
		if (SprintRunEnable)
		{
			WalkEnable = false;
			AimEnable = false;
			MovementState = EMovementState::SprintRun_State;
			AttackCharEvent(false);
		}
		else
		{
			if (!WalkEnable && !AimEnable)
			{
				MovementState = EMovementState::Run_State;
			}
			else if (WalkEnable && AimEnable)
			{
				MovementState = EMovementState::AimWalk_State;
			}
			else if (WalkEnable)
			{
				MovementState = EMovementState::Walk_State;
			}
			else if (AimEnable)
			{
				MovementState = EMovementState::Aim_State;
			}
		}
	}
	/*if (SprintRunEnable)
	{
		WalkEnable = false;
		AimEnable = false;
		MovementState = EMovementState::SprintRun_State;
		AttackCharEvent(false);
	}
	else
	{
		if (!WalkEnable && !AimEnable)
		{
			MovementState = EMovementState::Run_State;
		}
		else if (WalkEnable && AimEnable)
		{
			MovementState = EMovementState::AimWalk_State;		
		}
		else if (WalkEnable)
		{
			MovementState = EMovementState::Walk_State;
		}
		else if (AimEnable)
		{
			MovementState = EMovementState::Aim_State;
		}
	}*/
	CharacterUpdate();

	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		myWeapon->UpdateStateWeapon(MovementState);
	}
}

void AJunTDSCharacter::UpdateStateWeapon(EMovementState NewMovementState)
{
	ChangeDispersion();
}

void AJunTDSCharacter::ChangeDispersion()
{

}

AWeaponDefault* AJunTDSCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void AJunTDSCharacter::TryReloadWeapon()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
			CurrentWeapon->InitReload();
	}
}

void AJunTDSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	Anim = CurrentWeapon->WeaponSetting.AnimCharReload;
	GetMesh()->GetAnimInstance()->Montage_Play(Anim, 1.0f);
}

void AJunTDSCharacter::WeaponReloadEnd()
{
	ChangeDispersion();
}

