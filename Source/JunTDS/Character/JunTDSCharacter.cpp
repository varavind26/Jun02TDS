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

void AJunTDSCharacter::LookHorizontal(float value)
{
	if (bIsClimb || bIsMantle || bIsHurdle)
	{
		value = 0.f;
	}
	else
	{
		if (value != 0.f && Controller && Controller->IsLocalPlayerController())
		{
			APlayerController* const PC = CastChecked<APlayerController>(Controller);
			PC->AddYawInput(value);
		}
	}
}

void AJunTDSCharacter::LookVertical(float value)
{
	if (bIsClimb || SprintRunEnable || bIsMantle || bIsHurdle)
	{
		value = 0.f;
	}
	else
	{
		if (value != 0.f && Controller && Controller->IsLocalPlayerController())
		{
			APlayerController* const PC = CastChecked<APlayerController>(Controller);
			PC->AddPitchInput(value);
		}
	}
}

void AJunTDSCharacter::Movement()
{
	if (!bIsClimb || !bIsMantle || !bIsHurdle)
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

	NewInputComponent->BindAxis(TEXT("LookHorizontal"), this, &AJunTDSCharacter::LookHorizontal);
	NewInputComponent->BindAxis(TEXT("LookVertical"), this, &AJunTDSCharacter::LookVertical);

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
	if (ClimbingAnimation != nullptr && Traversal() && bIsClimb)
	{
		MovementState = EMovementState::Traversal_State;
		FRotator LastActorRotator = GetActorRotation();
		FVector LastActorLocation = GetActorLocation();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetActorLocation(FVector(WallHitLocationToClimb.X + WallHitNormalToClimb.X * WallOffset, WallHitLocationToClimb.Y + WallHitNormalToClimb.Y * WallOffset, (HeightHitLocationToClimb.Z - ClimbHandHeight)), true);
		SetActorRotation(LastActorRotator);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->Montage_Play(ClimbingAnimation, 1.0f);
	}
	else if (MantleAnimation != nullptr && Traversal() && bIsMantle)
	{
		MovementState = EMovementState::Traversal_State;
		FRotator LastActorRotator = GetActorRotation();
		FVector LastActorLocation = GetActorLocation();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetActorLocation(FVector(WallHitLocationToClimb.X + WallHitNormalToClimb.X * WallOffset, WallHitLocationToClimb.Y + WallHitNormalToClimb.Y * WallOffset, (HeightHitLocationToClimb.Z - MantleHandHeight)), true);
		SetActorRotation(LastActorRotator);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->Montage_Play(MantleAnimation, 1.0f);
	}
	else if (HurdleAnimation != nullptr && Traversal() && bIsHurdle)
	{
		MovementState = EMovementState::Traversal_State;
		FRotator LastActorRotator = GetActorRotation();
		FVector LastActorLocation = GetActorLocation();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetActorLocation(FVector(WallHitLocationToClimb.X + WallHitNormalToClimb.X * WallOffset, WallHitLocationToClimb.Y + WallHitNormalToClimb.Y * WallOffset, (WallHitLocationToClimb.Z + HurdleHandHeight)), true);
		SetActorRotation(LastActorRotator);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->Montage_Play(HurdleAnimation, 1.0f);
	}
}

void AJunTDSCharacter::OnClimbFinished(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ClimbingAnimation)
	{
		bIsClimb = false;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		MovementState = EMovementState::Aim_State;
	}
	if (Montage == MantleAnimation)
	{
		bIsMantle = false;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		MovementState = EMovementState::Aim_State;
	}
	if (Montage == HurdleAnimation)
	{
		bIsHurdle = false;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		MovementState = EMovementState::Aim_State;
	}
}

bool AJunTDSCharacter::Traversal()
{
	float CapsuleSize = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	FVector FwdTraceStart = GetActorLocation() - FVector(0.f, 0.f, CapsuleSize/2);
	FVector FwdTraceEnd = FwdTraceStart + (GetActorForwardVector() * DistToObject);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector PelvisLocation = GetMesh()->GetSocketLocation(TEXT("pelvisSocket"));

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, FwdTraceStart, FwdTraceEnd, ECC_Visibility, QueryParams);
	WallHitLocationToClimb = HitResult.Location;
	WallHitNormalToClimb = HitResult.Normal;
	AActor* HitActorFwd = HitResult.GetActor();
	UPrimitiveComponent* HitComponentFwd = HitResult.GetComponent();

	FVector TraceSize = FVector(0.f, 0.f, 500.f);
	FVector HeightTraceStart = WallHitLocationToClimb + TraceSize + GetActorForwardVector() * DistanceToHeightTrace;
	FVector HeightTraceEnd = HeightTraceStart - TraceSize;
	FHitResult HeightTraceHitResult;

	bool bHeightTraceHit = GetWorld()->LineTraceSingleByChannel(HeightTraceHitResult, HeightTraceStart, HeightTraceEnd, ECC_Visibility, QueryParams);
	HeightHitLocationToClimb = HeightTraceHitResult.Location;
	HeightHitNormalToClimb = HeightTraceHitResult.Normal;
	AActor* HitActorHeight = HeightTraceHitResult.GetActor();
	UPrimitiveComponent* HitComponentHeight = HeightTraceHitResult.GetComponent();

	float ToTraversalValue = HeightHitLocationToClimb.Z - PelvisLocation.Z;
	float ToHurdleValue = PelvisLocation.Z- WallHitLocationToClimb.Z;
	
	bool CheckHitObjectType = (HitActorFwd == HitActorHeight) || (HitComponentHeight == HitComponentFwd);
	bool CheckClimb = ((ToTraversalValue >= MinHeightToClimb) && (ToTraversalValue <= MaxHeightToClimb)) && CheckHitObjectType;
	bool CheckMantle = ((ToTraversalValue >= MinHeightToMantle) && (ToTraversalValue <= MaxHeightToMantle)) && CheckHitObjectType;
	bool CheckHurdle = ((ToHurdleValue >= MinHeightToHurdle) && (ToHurdleValue <= MaxHeightToHurdle)) && !CheckHitObjectType;
	
	const FName ClimbableTag = TEXT("Climbable");
	bool bHitClimbable = false;
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		bHitClimbable = (HitActorFwd && HitActor->ActorHasTag(ClimbableTag)) || (HitComponentFwd && HitComponent->ComponentHasTag(ClimbableTag));
	}

	bool TraversalSelect = CheckClimb || CheckMantle || CheckHurdle;

	if (TraversalSelect && (!bIsClimb || !bIsMantle || !bIsHurdle) && bHitClimbable)
	{
		DrawDebugLine(GetWorld(), FwdTraceStart, FwdTraceEnd, FColor::Red, false, 15.f, (uint8)'\000', 2.f);
		DrawDebugLine(GetWorld(), HeightTraceStart, HeightTraceEnd, FColor::Yellow, false, 15.f, (uint8)'\000', 2.f);
		if (TraversalSelect == CheckClimb)
		{
			UE_LOG(LogTemp, Warning, TEXT("Climb!"));
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			GetCharacterMovement()->StopMovementImmediately();
			bIsClimb = true;
		}
		else if (TraversalSelect == CheckMantle)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mantle!"));
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			GetCharacterMovement()->StopMovementImmediately();
			bIsMantle = true;
		}
		else if (TraversalSelect == CheckHurdle)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hurdle!"));
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			GetCharacterMovement()->StopMovementImmediately();
			bIsHurdle = true;
		}
		return true;
	}
	else
	{
		return false;
	}
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
	ChangeMovementState();
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

	if (bIsClimb || bIsMantle || bIsHurdle)
	{
		WalkEnable = false;
		AimEnable = false;
		SprintRunEnable = false;
		MovementState = EMovementState::Traversal_State;
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

