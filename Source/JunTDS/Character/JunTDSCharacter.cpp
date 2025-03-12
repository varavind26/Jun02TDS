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

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetActorLocation(FVector(WallHitLocationToClimb.X+WallHitNormalToClimb.X*WallOffset, WallHitLocationToClimb.Y+WallHitNormalToClimb.Y * WallOffset, (HeightHitLocationToClimb.Z - HandUpperPostitionToClimb)),true);
		SetActorRotation(LastActorRotator);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->OnMontageEnded.AddDynamic(this, &AJunTDSCharacter::OnClimbFinished);
		AnimInstance->Montage_Play(ClimbingAnimation, 1.0f);
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
	FVector FwdTraceStart = GetActorLocation();
	FVector FwdTraceEnd = FwdTraceStart + (GetActorForwardVector() * DistToObject);
	FHitResult HitResult;

	FVector TraceSize = FVector(0.f, 0.f, 500.f);
	FVector HeightTraceStart = GetActorLocation() + TraceSize + GetActorForwardVector() * DistanceToHeightTrace;
	FVector HeightTraceEnd = HeightTraceStart - TraceSize;
	FHitResult HeightTraceHitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FVector PelvisLocation = GetMesh()->GetSocketLocation(TEXT("pelvisSocket"));

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, FwdTraceStart, FwdTraceEnd, ECC_Visibility, QueryParams);
	WallHitLocationToClimb = HitResult.Location;
	WallHitNormalToClimb = HitResult.Normal;
	bool bHeightTraceHit = GetWorld()->LineTraceSingleByChannel(HeightTraceHitResult, HeightTraceStart, HeightTraceEnd, ECC_Visibility, QueryParams);
	HeightHitLocationToClimb = HeightTraceHitResult.Location;
	HeightHitNormalToClimb = HeightTraceHitResult.Normal;
	float ToClimbValue = (HeightHitLocationToClimb.Z - PelvisLocation.Z);
	bool CheckClimb = ((ToClimbValue >= MinHeightToClimb) && (ToClimbValue <= MaxHeightToClimb));
	

	float VecSize = 500.f;

	FHitResult AdditionalHitResult;
	bool bAdditionalHit = GetWorld()->LineTraceSingleByChannel(AdditionalHitResult,WallHitLocationToClimb,WallHitLocationToClimb+ HeightHitNormalToClimb* VecSize,ECC_Visibility,QueryParams);
	FVector AdditionalHitLocation;
	FVector AdditionalHitNormal;
	if (bAdditionalHit)
	{
		AdditionalHitLocation = AdditionalHitResult.Location;
		AdditionalHitNormal = AdditionalHitResult.Normal;
	}

	FHitResult SecondAdditionalHitResult;
	bool bSecondAdditionalHit = GetWorld()->LineTraceSingleByChannel(SecondAdditionalHitResult, HeightHitLocationToClimb,HeightHitLocationToClimb + WallHitNormalToClimb * VecSize,ECC_Visibility,QueryParams);

	FVector SecondAdditionalHitLocation;
	FVector SecondAdditionalHitNormal;

	if (bSecondAdditionalHit)
	{
		SecondAdditionalHitLocation = SecondAdditionalHitResult.Location;
		SecondAdditionalHitNormal = SecondAdditionalHitResult.Normal;
	}

	// Теперь находим пересечение двух дополнительных трасс
	FVector IntersectionPoint;
	bool IsIntersectionFound = FindIntersectionPoint(WallHitLocationToClimb, WallHitLocationToClimb + HeightHitNormalToClimb * VecSize,HeightHitLocationToClimb, HeightHitLocationToClimb + WallHitNormalToClimb * VecSize,IntersectionPoint);

	if (IsIntersectionFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Точка пересечения найдена: %s"), *IntersectionPoint.ToString());
	}

	// Проверка тегов для объектов трейсов
	const FName ClimbableTag = TEXT("Climbable");
	bool bHitClimbable = false;
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		bHitClimbable = (HitActor && HitActor->ActorHasTag(ClimbableTag)) || (HitComponent && HitComponent->ComponentHasTag(ClimbableTag));
	}

	bool bHeightClimbable = false;
	if (bHeightTraceHit)
	{
		AActor* HeightActor = HeightTraceHitResult.GetActor();
		UPrimitiveComponent* HeightComponent = HeightTraceHitResult.GetComponent();
		bHeightClimbable = (HeightActor && HeightActor->ActorHasTag(ClimbableTag)) || (HeightComponent && HeightComponent->ComponentHasTag(ClimbableTag));
	}

	if (bHeightTraceHit && bHit && CheckClimb && !bIsClimb && bHitClimbable && bHeightClimbable)
	{
		DrawDebugLine(GetWorld(), FwdTraceStart, FwdTraceEnd, FColor::Red, false, 15.f, (uint8)'\000', 2.f);
		DrawDebugLine(GetWorld(), HeightTraceStart, HeightTraceEnd, FColor::Yellow, false, 15.f, (uint8)'\000', 2.f);
		DrawDebugLine(GetWorld(), WallHitLocationToClimb, WallHitLocationToClimb + HeightHitNormalToClimb * VecSize, FColor::Green, false, 15.f, (uint8)'\000', 2.f);
		DrawDebugLine(GetWorld(), HeightHitLocationToClimb, HeightHitLocationToClimb + WallHitNormalToClimb * VecSize, FColor::Blue, false, 15.f, (uint8)'\000', 2.f);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->StopMovementImmediately();
		return true;
	}
	else
	{
		return false;
	}
}

bool AJunTDSCharacter::FindIntersectionPoint(const FVector& A1, const FVector& A2, const FVector& B1, const FVector& B2, FVector& OutIntersectionPoint)
{
	float Ax1 = A1.X;
	float Ay1 = A1.Y;
	float Ax2 = A2.X;
	float Ay2 = A2.Y;

	float Bx1 = B1.X;
	float By1 = B1.Y;
	float Bx2 = B2.X;
	float By2 = B2.Y;

	// Вычисляем коэффициенты уравнений прямой
	float Denominator = (Bx2 - Bx1) * (Ay2 - Ay1) - (By2 - By1) * (Ax2 - Ax1);

	if (std::abs(Denominator) < 1e-6f) // Проверяем, чтобы прямые не были параллельны
		return false;

	float NumeratorA = (Bx2 - Bx1) * (Ay1 - By1) - (By2 - By1) * (Ax1 - Bx1);
	float NumeratorB = (Ax2 - Ax1) * (Ay1 - By1) - (Ay2 - Ay1) * (Ax1 - Bx1);

	float Ua = NumeratorA / Denominator;
	float Ub = NumeratorB / Denominator;

	// Проверяем, лежат ли параметры Ua и Ub внутри отрезков
	if (Ua < 0 || Ua > 1 || Ub < 0 || Ub > 1)
		return false;

	// Нахождение точки пересечения
	OutIntersectionPoint = FVector(Ax1 + Ua * (Ax2 - Ax1), Ay1 + Ua * (Ay2 - Ay1), 0.f);

	return true;
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

