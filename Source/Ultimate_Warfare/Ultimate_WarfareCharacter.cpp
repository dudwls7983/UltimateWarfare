// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Ultimate_WarfareCharacter.h"
#include "Ultimate_WarfareProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId

#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AUltimate_WarfareCharacter

AUltimate_WarfareCharacter::AUltimate_WarfareCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 60.f);
	FirstPersonCameraComponent->RelativeRotation = FRotator(0.f, 90.f, 0.f);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeLocation = FVector(2.5f, 0.13f, -165.45f);
	Mesh1P->RelativeRotation = FRotator(0.f, -90.f, 0.f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(Mesh1P);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	CameraCurveFloat = CreateDefaultSubobject<UCurveFloat>(TEXT("AimDownSightCurve"));
	CameraCurveFloat->FloatCurve.AddKey(0.f, 0.f);
	CameraCurveFloat->FloatCurve.AddKey(0.15f, 1.f);

	PrimaryActorTick.bCanEverTick = true;
	
	FOnTimelineFloat ADSInterpFunction;
	ADSInterpFunction.BindUFunction(this, FName("InterpADSFOV"));

	ADSTimeline = FTimeline();
	ADSTimeline.AddInterpFloat(CameraCurveFloat, ADSInterpFunction, TEXT("InterpValue"));

	FOnTimelineFloat SprintInterpFunction;
	SprintInterpFunction.BindUFunction(this, FName("InterpSprintFOV"));

	SprintTimeline = FTimeline();
	SprintTimeline.AddInterpFloat(CameraCurveFloat, SprintInterpFunction, TEXT("InterpValue"));
}

void AUltimate_WarfareCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("R_GunSocket"));

	Mesh1P->SetHiddenInGame(false, true);
}

void AUltimate_WarfareCharacter::Tick(float delta)
{
	Super::Tick(delta);
	if (ADSTimeline.IsPlaying())
	{
		ADSTimeline.TickTimeline(delta);
	}
	if (SprintTimeline.IsPlaying())
	{
		SprintTimeline.TickTimeline(delta);
	}
	if (isFire)
	{
		// 아래 행동 중에 총을 발사할 수 없다.
		if (isSprint) return;

		float time = GetWorld()->GetTimeSeconds();
		if (time > nextShootTime)
		{
			nextShootTime = time + fireRate;
			OnFire();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUltimate_WarfareCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AUltimate_WarfareCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AUltimate_WarfareCharacter::EndFire);

	// Bind aim event
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AUltimate_WarfareCharacter::ToggleAimDownSight);

	// Bind sprint event
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AUltimate_WarfareCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AUltimate_WarfareCharacter::EndSprint);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUltimate_WarfareCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AUltimate_WarfareCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUltimate_WarfareCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AUltimate_WarfareCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AUltimate_WarfareCharacter::LookUpAtRate);
}

void AUltimate_WarfareCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AUltimate_WarfareProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	TArray<UAnimMontage *> fireAnimation = isADS ? FireIronSightAnimation : FireAnimation;
	int count = fireAnimation.Num();
	// try and play a firing animation if specified
	if (count > 0)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(fireAnimation[FMath::RandRange(0, count - 1)], 1.f);
		}
		UAnimInstance* AnimInstance2 = FP_Gun->GetAnimInstance();
		if (AnimInstance2 != NULL && RifleShootAnimation != NULL)
		{
			AnimInstance2->Montage_Play(RifleShootAnimation, 1.f);
		}
	}
}

void AUltimate_WarfareCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUltimate_WarfareCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AUltimate_WarfareCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AUltimate_WarfareCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUltimate_WarfareCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AUltimate_WarfareCharacter::ToggleAimDownSight()
{
	// 아래 행동 중에는 aim down sight를 할 수 없다.
	if (isSprint || isFire) return;

	isADS = !isADS;
	if (isADS)
		ADSTimeline.Play();
	else
		ADSTimeline.Reverse();
}

void AUltimate_WarfareCharacter::InterpADSFOV(float interp)
{
	FirstPersonCameraComponent->FieldOfView = FMath::Lerp<float, float>(90.f, 75.f, interp);
}

void AUltimate_WarfareCharacter::BeginSprint()
{
	// 아래 행동 중에는 뛸 수 없다.
	if (isADS || isFire) return;

	isSprint = true;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	SprintTimeline.Play();
}

void AUltimate_WarfareCharacter::EndSprint()
{
	if (isADS || isFire) return;

	isSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	SprintTimeline.Reverse();
}

void AUltimate_WarfareCharacter::InterpSprintFOV(float interp)
{
	FirstPersonCameraComponent->FieldOfView = FMath::Lerp<float, float>(90.f, 100.f, interp);
}

void AUltimate_WarfareCharacter::BeginFire()
{
	// isReloading = true then return
	isFire = true;
}

void AUltimate_WarfareCharacter::EndFire()
{
	isFire = false;
}
