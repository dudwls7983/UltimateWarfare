// Fill out your copyright notice in the Description page of Project Settings.

#include "Attack.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

CREATE_STATE_CPP(UAttack)


void UAttack::OnEnter(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UAttack::OnEnter"));

	// Set next shoot time and pickup weapon
	owner->SetNextShootTime(owner->GetWorld()->GetTimeSeconds() + 1.f);
}


void UAttack::OnUpdate(AUltimate_WarfareEnemy *owner)
{
	AActor *target = owner->GetTarget();
	if (target == nullptr)
	{
		return;
	}
	if (owner->CanSeeTarget() == false)
	{
		owner->SetState(owner->stateMap[EEnemyState::ES_Chase]);
		return;
	}

	// µµÂøÁöÁ¡Àº Å¸°ÙÀÇ À§Ä¡
	FVector endLocation = target->GetActorLocation();
	// º¿ -> Å¸°Ù º¤ÅÍ »ý¼º
	FVector shootVector = endLocation - owner->GetFirstPersonCameraComponent()->GetComponentLocation();
	FRotator shootAngle = shootVector.Rotation();
	owner->GetFirstPersonCameraComponent()->SetWorldRotation(shootAngle);
	shootAngle.Pitch = 0.f;
	owner->SetActorRotation(shootAngle);

	// ¹ß»ç °¡´É
	if (owner->GetWorld()->GetTimeSeconds() - owner->GetNextShootTime() > 0)
	{
		owner->Fire();
		owner->SetNextShootTime(owner->GetWorld()->GetTimeSeconds() + owner->fireRate * FMath::RandRange(1, 5));
	}
}


void UAttack::OnExit(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UAttack::OnExit"));
}


