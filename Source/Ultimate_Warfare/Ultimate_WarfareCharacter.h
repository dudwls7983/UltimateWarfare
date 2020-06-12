// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"

#include "Ultimate_WarfareCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AUltimate_WarfareCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool isADS; // Aim Down Sight�� ������ΰ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool isSprint; // �ٴ� ���ΰ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool isFire; // �߻� ���ΰ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	bool isCrouch; // �ɴ� ���ΰ�

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCurveFloat *CameraCurveFloat; // ī�޶� FOV�� ��ȭ�� ���� Curve Float

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCurveFloat *RecoilCurveFloat; // Recoil �����̸� ���� Curve Float

	FTimeline ADSTimeline; // Aim Down Sight�� Ÿ�Ӷ���
	FTimeline SprintTimeline; // Sprint�� Ÿ�Ӷ���
	FTimeline CrouchTimeline; // Crouch�� Ÿ�Ӷ���
	FTimeline RecoilTimeline; // Recoil�� Ÿ�Ӷ���

	float nextShootTime; // ���� �߻� ���� �ð�

	FVector CameraRelativeLocation;
	FRotator CameraRecoilVector = FRotator::ZeroRotator;
	FRotator PreviousRecoilVector = FRotator::ZeroRotator;

public:
	AUltimate_WarfareCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float delta) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<class UAnimMontage*> FireAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<class UAnimMontage*> FireIronSightAnimation;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* RifleShootAnimation;

	/** need time to fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float fireRate = 0.1f;

	/** recoil of weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float recoilRate = 0.3f;

	/** effective range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float maxDistance = 4000.f;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// ��Ŭ������ AimDownSight ���
	void ToggleAimDownSight();

	UFUNCTION()
	void InterpADSFOV(float interp);

	void BeginSprint();
	void EndSprint();

	UFUNCTION()
	void InterpSprintFOV(float interp);

	void BeginFire();
	void EndFire();

	void BeginCrouch();
	void EndCrouch();

	UFUNCTION()
	void InterpCrouch(float interp);

	UFUNCTION()
	void InterpRecoil(float interp);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void TestFunction();

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

