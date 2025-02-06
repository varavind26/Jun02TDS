// Copyright Epic Games, Inc. All Rights Reserved.

#include "JunTDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "JunTDS/Game/JunTDSGameInstance.h"

AJunTDSCharacter::AJunTDSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
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
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
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

void AJunTDSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon(InitWeaponName);
}

void AJunTDSCharacter::InitWeapon(FName IdWeaponName)
{
	UJunTDSGameInstance* myGI = Cast<UJunTDSGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;
	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
		{
			if (myWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
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
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
	}
}

void AJunTDSCharacter::SetupPlayerInputComponent(UInputComponent* NewInputComponent)
{
	Super::SetupPlayerInputComponent(NewInputComponent);

	NewInputComponent->BindAxis(TEXT("MoveForward"), this, & AJunTDSCharacter::InputAxisX);
	NewInputComponent->BindAxis(TEXT("MoveRight"), this, & AJunTDSCharacter::InputAxisY);

	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::InputAttackPressed);
	NewInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::InputAttackReleased);

	NewInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::OnSprintPressed);
	NewInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &AJunTDSCharacter::OnSprintReleased);

	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Pressed, this, &AJunTDSCharacter::OnAimPressed);
	NewInputComponent->BindAction(TEXT("AimEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::OnAimReleased);

	NewInputComponent->BindAction(TEXT("ReloadEvent"), EInputEvent::IE_Released, this, &AJunTDSCharacter::TryReloadWeapon);
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

void AJunTDSCharacter::InputAxisX(float Value)
{
	if (SprintRunEnable && Value <= 0.0f)
	{
		OnSprintReleased();
	}

	AxisX = SprintRunEnable ? FMath::Clamp(Value, 0.0f, 1.0f) : Value;
}

void AJunTDSCharacter::InputAxisY(float Value)
{
	AxisY = SprintRunEnable ? 0.0f : Value;
}

void AJunTDSCharacter::InputAttackPressed()
{
	AttackCharEvent(true);
}

void AJunTDSCharacter::InputAttackReleased()
{
	AttackCharEvent(false);
}

void AJunTDSCharacter::MovementTick(float DeltaTime)
{
	APlayerController* myController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (myController)
	{
		// Поворот персонажа к курсору
		FHitResult ResultHit;
		myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);
		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

		// Расчет векторов движения
		FVector ForwardDirection = (ResultHit.Location - GetActorLocation()).GetSafeNormal();
		ForwardDirection.Z = 0;
		ForwardDirection.Normalize();
		FVector StrafeDirection = UKismetMathLibrary::RotateAngleAxis(ForwardDirection, 90.0f, FVector::UpVector);

		if (SprintRunEnable)
		{
			AddMovementInput(ForwardDirection, FMath::Clamp(AxisX, 0.0f, 1.0f));
		}
		else
		{
			AddMovementInput(ForwardDirection, AxisX);
			AddMovementInput(StrafeDirection, AxisY);

			if (CurrentWeapon)
			{
				FVector Displacement = FVector(0);
				switch (MovementState)
				{
				case EMovementState::Aim_State:
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					CurrentWeapon->ShouldReduceDispersion = true;
					break;
				case EMovementState::AimWalk_State:
					CurrentWeapon->ShouldReduceDispersion = true;
					Displacement = FVector(0.0f, 0.0f, 160.0f);
					break;
				case EMovementState::Walk_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				case EMovementState::Run_State:
					Displacement = FVector(0.0f, 0.0f, 120.0f);
					CurrentWeapon->ShouldReduceDispersion = false;
					break;
				case EMovementState::SprintRun_State:
					break;
				default:
					break;
				}

				CurrentWeapon->ShootEndLocation = ResultHit.Location + Displacement;
			}
		}
	}
};

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

}

