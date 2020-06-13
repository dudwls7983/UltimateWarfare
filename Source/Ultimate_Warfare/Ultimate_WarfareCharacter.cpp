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

#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

	GetMesh()->RelativeLocation = FVector(0.f, 0.f, -100.f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 160.f);
	FirstPersonCameraComponent->RelativeRotation = FRotator(0.f, 90.f, 0.f);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeLocation = FVector(2.5f, 0.13f, -165.5f);
	Mesh1P->RelativeRotation = FRotator(0.f, -90.f, 0.f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(Mesh1P);
	FP_Gun->RelativeRotation = FRotator(0.f, 0.f, 0.2f);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	PrimaryActorTick.bCanEverTick = true;

	CameraCurveFloat = NewObject<UCurveFloat>();
	CameraCurveFloat->FloatCurve.Reset();
	CameraCurveFloat->FloatCurve.AddKey(0.f, 0.f);
	CameraCurveFloat->FloatCurve.AddKey(0.15f, 1.f);
	CameraCurveFloat->AddToRoot();

	FOnTimelineFloat ADSInterpFunction;
	ADSInterpFunction.BindUFunction(this, FName("InterpADSFOV"));

	ADSTimeline = FTimeline();
	ADSTimeline.AddInterpFloat(CameraCurveFloat, ADSInterpFunction, TEXT("InterpValue"));

	FOnTimelineFloat SprintInterpFunction;
	SprintInterpFunction.BindUFunction(this, FName("InterpSprintFOV"));

	SprintTimeline = FTimeline();
	SprintTimeline.AddInterpFloat(CameraCurveFloat, SprintInterpFunction, TEXT("InterpValue"));

	FOnTimelineFloat CrouchInterpFunction;
	CrouchInterpFunction.BindUFunction(this, FName("InterpCrouch"));

	CrouchTimeline = FTimeline();
	CrouchTimeline.AddInterpFloat(CameraCurveFloat, CrouchInterpFunction, TEXT("InterpValue"));

	RecoilCurveFloat = NewObject<UCurveFloat>();
	RecoilCurveFloat->FloatCurve.Reset();
	RecoilCurveFloat->FloatCurve.AddKey(0.f, 0.f);
	RecoilCurveFloat->FloatCurve.AddKey(0.2f, 0.f);
	RecoilCurveFloat->FloatCurve.AddKey(0.3f, 1.f);
	RecoilCurveFloat->AddToRoot();

	FOnTimelineFloat RecoilInterpFunction;
	RecoilInterpFunction.BindUFunction(this, FName("InterpRecoil"));

	RecoilTimeline = FTimeline();
	RecoilTimeline.AddInterpFloat(RecoilCurveFloat, RecoilInterpFunction, TEXT("InterpValue"));
}

void AUltimate_WarfareCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("R_GunSocket"));

	Mesh1P->SetHiddenInGame(false, true);

	CameraRelativeLocation = FirstPersonCameraComponent->GetRelativeTransform().GetLocation();
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
	if (CrouchTimeline.IsPlaying())
	{
		CrouchTimeline.TickTimeline(delta);
	}
	if (RecoilTimeline.IsPlaying())
	{
		RecoilTimeline.TickTimeline(delta);
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

	// Bind crouch event
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AUltimate_WarfareCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AUltimate_WarfareCharacter::EndCrouch);

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
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		//const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		//const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		//Set Spawn Collision Handling Override
		//FActorSpawnParameters ActorSpawnParams;
		//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		//World->SpawnActor<AUltimate_WarfareProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

		APlayerCameraManager *camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

		const FVector EyePosition = camManager->GetCameraLocation();
		FRotator EyeRotation = camManager->GetCameraRotation();
		float randFloat = FMath::FRandRange(0, PI);
		float inaccuracy = FMath::FRandRange(0, isADS ? inAccuracy : inAccuracy * 3);
		EyeRotation.Pitch -= FMath::Sin(randFloat) * inaccuracy;
		EyeRotation.Yaw += FMath::Cos(randFloat) * inaccuracy;
		const FVector EndLocation = EyePosition + EyeRotation.Vector() * maxDistance;
		FVector force = (EndLocation - EyePosition).GetSafeNormal();

		FHitResult hitResult;
		GetWorld()->LineTraceSingleByChannel(hitResult, EyePosition, EndLocation, ECollisionChannel::ECC_Visibility);

		DrawDebugLine(GetWorld(), EyePosition, hitResult.Location, FColor::Red, false, 10.f, 0, 0.5f);

		AActor *hitObject = hitResult.GetActor();
		if (hitObject != NULL)
		{
			TArray<AActor*> ignoredActors;
			UGameplayStatics::ApplyPointDamage(hitObject, 100.f, force, hitResult, GetWorld()->GetFirstPlayerController(), this, UDamageType::StaticClass());
		}

		// 움직일 수 있는 오브젝트는 힘을 가한다.
		UPrimitiveComponent *hitComponent = hitResult.GetComponent();
		if (hitComponent != NULL && hitComponent->Mobility == EComponentMobility::Movable && hitComponent->IsSimulatingPhysics() == true)
		{
			hitComponent->AddImpulseAtLocation(force * 100000.f, hitResult.Location);
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

	if (recoilRate > 0.f)
	{
		/*AddControllerPitchInput(-recoilRate);
		AddControllerYawInput(FMath::FRandRange(-recoilRate, recoilRate));
		UE_LOG(LogTemp, Display, TEXT("%s"), *FirstPersonCameraComponent->RelativeRotation.ToString());*/
		CameraRecoilVector.Pitch -= recoilRate;
		CameraRecoilVector.Pitch = FMath::Max(CameraRecoilVector.Pitch, recoilRate * -5);

		CameraRecoilVector.Yaw += FMath::FRandRange(-recoilRate, recoilRate);
		CameraRecoilVector.Yaw = FMath::Clamp(CameraRecoilVector.Yaw, recoilRate * -5, recoilRate * 5);

		FRotator newRotation = CameraRecoilVector - PreviousRecoilVector;

		AddControllerPitchInput(newRotation.Pitch);
		AddControllerYawInput(newRotation.Yaw);

		PreviousRecoilVector = CameraRecoilVector;

		RecoilTimeline.PlayFromStart();
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
	FirstPersonCameraComponent->FieldOfView = FMath::Lerp<float, float>(90.f, 60.f, interp);
}

void AUltimate_WarfareCharacter::BeginSprint()
{
	// 아래 행동 중에는 뛸 수 없다.
	if (isADS || isFire || isCrouch) return;

	isSprint = true;
	GetCharacterMovement()->MaxWalkSpeed = 750.f;
	SprintTimeline.Play();
}

void AUltimate_WarfareCharacter::EndSprint()
{
	if (isADS || isFire || isCrouch) return;

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

void AUltimate_WarfareCharacter::BeginCrouch()
{
	// 달리는 중에 앉을 수 없다.
	if (isSprint) return;

	isCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	CrouchTimeline.Play();
}

void AUltimate_WarfareCharacter::EndCrouch()
{
	if (isSprint) return;

	isCrouch = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	CrouchTimeline.Reverse();
}

void AUltimate_WarfareCharacter::InterpCrouch(float interp)
{
	GetCapsuleComponent()->SetCapsuleHalfHeight(FMath::Lerp<float, float>(88.f, 44.f, interp));

	FVector ToCameraLocation = CameraRelativeLocation;
	ToCameraLocation.Z -= 20.f;
	FirstPersonCameraComponent->SetRelativeLocation(FMath::Lerp<FVector, float>(CameraRelativeLocation, ToCameraLocation, interp));
}

void AUltimate_WarfareCharacter::InterpRecoil(float interp)
{
	CameraRecoilVector = FMath::Lerp<FRotator, float>(CameraRecoilVector, FRotator::ZeroRotator, interp);

	FRotator newRotation = CameraRecoilVector - PreviousRecoilVector;

	AddControllerPitchInput(newRotation.Pitch);
	AddControllerYawInput(newRotation.Yaw);

	PreviousRecoilVector = CameraRecoilVector;
}
